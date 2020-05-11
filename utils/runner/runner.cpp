/* From https://github.com/PisonJay/botzone-local-runner/blob/master/runner.cpp
 */

#include <algorithm>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <json/json.h>
#include <sstream>
#include <string>
#include <unistd.h>
using namespace std;

#define err_sys(...) fprintf(stderr, __VA_ARGS__), exit(-1)

const int BUFFER_SIZE = 5e4;
char buf[BUFFER_SIZE];

const char *bot_path[] = {"./bot", "./bot", "./bot", "./bot"};
const char *judger_path = "./judger";
const char *log_file = "log.json";

#define BOT_ENDING ">>>BOTZONE_REQUEST_KEEP_RUNNING<<<"
const int BOT_ENDING_LEN = strlen(BOT_ENDING);

bool quiet_mode = false;

bool check_end(const char *s) {
    const char *t = BOT_ENDING;
    while (*t) {
        if ((*s++) != (*t++))
            return false;
    }
    return true;
}
struct subproc_t {
    pid_t pid;
    int fd[2];

    void Write(const char *s) {
        // puts(">>>>>");
        // puts(s);
        // puts("<<<<<");
        write(fd[1], s, strlen(s));
    }
    void Read(bool is_bot = false, char *buffer = buf,
              size_t len = BUFFER_SIZE) {
        // read to buf
        int pos = read(fd[0], buffer, len), last_pos = 0;
        if (is_bot) {
            bool flag = false;
            while (!flag) {
                for (int i = last_pos; i < pos - BOT_ENDING_LEN && !flag; i++)
                    if (check_end(buf + i))
                        flag = true;
                if (!flag)
                    pos += read(fd[0], buffer + (last_pos = pos), len - pos);
            }
        } else {
            while ((pos += read(fd[0], buffer + (last_pos = pos), len - pos)) !=
                   last_pos)
                ;
        }
        buffer[pos] = 0;
    }
    void Close() { close(fd[1]); }
    void Kill(int sig) { kill(pid, sig); }
};
void init(const char *prog_name, subproc_t &proc) {
    pid_t &pid = proc.pid;
    int fd[2][2];
    if (pipe(fd[0]) < 0 || pipe(fd[1]) < 0)
        err_sys("pipe error");
    if ((pid = fork()) < 0) {
        err_sys("fork error");
    } else if (pid > 0) {
        // parent
        close(fd[1][1]);
        close(fd[0][0]);
        proc.fd[0] = fd[1][0];
        proc.fd[1] = fd[0][1];
        return;
    } else {
        // child
        close(fd[1][0]);
        close(fd[0][1]);
        if (fd[0][0] != STDIN_FILENO)
            if (dup2(fd[0][0], STDIN_FILENO) != STDIN_FILENO)
                err_sys("dup2 error to stdin");
        if (fd[1][1] != STDOUT_FILENO)
            if (dup2(fd[1][1], STDOUT_FILENO) != STDOUT_FILENO)
                err_sys("dup2 error to stdout");
        execlp(prog_name, prog_name, (char *)0);
    }
}

subproc_t bots[4], judger;
#define JUDGER_INIT_STR "{}"

Json::Value initdata;
Json::Reader reader;
Json::FastWriter writer;

Json::Value outputValue, botValue;
Json::Value judger_content;

void init_judger() {
    init(judger_path, judger);
    judger.Write(JUDGER_INIT_STR);
    judger.Close();
    judger.Read();
    Json::Value val, tmp;
    reader.parse(buf, val);
    initdata = val["initdata"];
    judger_content = val["content"];
    val.removeMember("initdata");
    tmp["output"] = val;

    outputValue["log"].append(tmp);
    outputValue["initdata"] = initdata;
}

void run_bot(int i, bool is_init = false) {
    if (is_init) {
        init(bot_path[i], bots[i]);
        bots[i].Write(
            ("1\n" + judger_content[to_string(i)].asString() + "\n\n\n")
                .c_str());
    } else {
        bots[i].Kill(SIGCONT);
        bots[i].Write((judger_content[to_string(i)].asString() + "\n").c_str());
    }

    bots[i].Read(true);
    bots[i].Kill(SIGSTOP);

    stringstream sin(buf);
    string response, debug_message;
    getline(sin, response);
    getline(sin, debug_message);
    botValue[to_string(i)]["verdict"] = "OK";
    botValue[to_string(i)]["response"] = response;
    botValue[to_string(i)]["debug"] = debug_message;
}

int main(int argc, const char *argv[]) {

    Json::Value val, tmp;
    init_judger();

    if (argc > 1) {
        if (!strcmp(argv[1], "--help") || !strcmp(argv[1], "-h")) {
            puts("usage:");
            puts("\t-h/--help show this message");
            puts("\t-q/--quiet enable quiet mode, output final_output only");
            exit(0);
        } else if (!strcmp(argv[1], "--quiet") || !strcmp(argv[1], "-q")) {
            quiet_mode = true;
        }
    }

    if (!quiet_mode) {
        puts("initdata:");
        printf(writer.write(initdata).c_str());
    }
    for (int i = 0; i < 4; i++)
        run_bot(i, true);
    outputValue["log"].append(botValue);

    while (true) {
        init(judger_path, judger);
        judger.Write(writer.write(outputValue).c_str());
        judger.Close();
        judger.Read();
        reader.parse(buf, val);

        botValue.clear();
        if (val["command"] == "request") {
            tmp["output"] = val;
            outputValue["log"].append(tmp);
            judger_content = val["content"];
            for (int i = 0; i < 4; i++)
                run_bot(i);
            outputValue["log"].append(botValue);
        } else {
            if (!quiet_mode)
                puts("final_output:");
            printf(writer.write(val).c_str());
            freopen(log_file, "w", stdout);
            printf(writer.write(outputValue).c_str());
            break;
        }
    }
    for (int i = 0; i < 4; i++)
        bots[i].Kill(SIGKILL);
}