#include <netinet/in.h>
#include <sys/socket.h>

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <stack>
#include <string>
#include <unordered_map>

#include "account.h"
#include "error.h"
#include "filemap.hpp"
#include "lib.h"
#include "log.h"
#include "parser.h"

#define mkpr std::pair<Account, Book>
std::vector<mkpr> user_stack;
MainInfo<Account> account_info("account_info");
LogForAll log_all;
const std::string kMod = "*-4980(2jofw0.39ac2s@&";
std::string Process(std::string cmd) {
    // freopen("test.in", "r", stdin);
    // cin.tie(0);
    // std::cout.tie(0);
    std::string process_line(TokenScanner & line);
    if (cmd == "") return "";
    // if (line == "exit" || line == "quit") break;
    TokenScanner buffer(cmd);
    try {
        for (int i = 0; i < cmd.length(); i++)
            if ((cmd[i] < 32 && cmd[i] != 10 && cmd[i] != 13) ||
                cmd[i] >= 127 || !isascii(cmd[i]))
                throw Error();
        if (cmd.length() > 1024) throw Error();
        return process_line(buffer);
    } catch (Error &ex) {
        return ex.what();
    }
}

std::string process_line(TokenScanner &line) {
    std::string token;
    token = line.nextToken();
    if (token == kMod)
        return "";
    else if (token == "exit" || token == "quit") {
        if (line.nextToken() != kMod)
            throw Error();
        else
            exit(0);
    } else if (token == "su") {
        Account tmp;
        std::string user_id = line.nextToken(), password = line.nextToken();
        if (line.nextToken() != kMod) throw Error();
        if (IfInvaild(user_id.c_str(), 1, 30) ||
            IfInvaild(password.c_str(), 1, 30))
            throw Error();
        tmp = account_info.FindInfo(user_id);
        if (password == kMod) {  // may have problems
            if (user_stack.empty()) throw Error();
            if (user_stack.rbegin()->first.GetPriority() <= tmp.GetPriority())
                throw Error();
        } else if (tmp.GetPasswd() != password)
            throw Error();
        user_stack.push_back(mkpr(tmp, Book()));
        log_all.Record("0 0 login " + user_id);
        return "1";
    } else if (token == "logout") {
        if (line.nextToken() != kMod) throw Error();
        if (user_stack.empty()) throw Error();
        log_all.Record("0 0 logout " + user_stack.rbegin()->first.GetUserId());
        user_stack.pop_back();
        if (!user_stack.empty() && user_stack.rbegin()->second.GetISBN() != "")
            user_stack.rbegin()->second =
                Select(user_stack.rbegin()->second.GetIndex());
        return "1";
    } else if (token == "register") {
        std::string user_id, password, user_name;
        user_id = line.nextToken(), password = line.nextToken(),
        user_name = line.nextToken();
        if (IfInvaild(user_id.c_str(), 1, 30) ||
            IfInvaild(password.c_str(), 1, 30) ||
            IfInvaild(password.c_str(), 2, 30))
            throw Error();
        if (line.nextToken() != kMod || user_name == kMod) throw Error();
        Register(user_id, password, user_name);
        log_all.Record("0 0 register " + user_id);
        return "1";
    } else if (token == "passwd") {
        //`passwd [User-ID] ([Old-Password])? [New-Password]`
        std::string user_id, old_password, new_password;
        user_id = line.nextToken(), old_password = line.nextToken(),
        new_password = line.nextToken();
        if (user_stack.empty()) throw Error();
        if (line.nextToken() != kMod) throw Error();
        if (old_password == kMod && new_password == kMod) throw Error();
        if (old_password != kMod && new_password == kMod &&
            (user_stack.empty() ||
             user_stack.rbegin()->first.GetPriority() != 7))
            throw Error();
        if (IfInvaild(user_id.c_str(), 1, 30) ||
            IfInvaild(old_password.c_str(), 1, 30) ||
            IfInvaild(new_password.c_str(), 1, 30))
            throw Error();
        int priority;
        if (user_stack.empty())
            priority = 0;
        else
            priority = user_stack.rbegin()->first.GetPriority();
        if (user_stack.empty() ||
            user_stack.rbegin()->first.GetPriority() != 7 ||
            new_password != kMod)
            Passwd(user_id, new_password, priority, old_password);
        else
            Passwd(user_id, old_password, priority, "");
        log_all.Record(user_stack.rbegin()->first.GetUserId() + " " +
                       std::to_string(user_stack.rbegin()->first.GetPriority()) +
                       " passwd " + user_id);
        return "1";
    } else if (token == "useradd") {
        //`useradd [User-ID] [Password] [Priority] [User-Name]`
        std::string user_id, password, priority, user_name;
        user_id = line.nextToken(), password = line.nextToken(),
        priority = line.nextToken(), user_name = line.nextToken();
        if (line.nextToken() != kMod || user_name == kMod ||
            priority.length() != 1)
            throw Error();
        if (IfInvaild(user_id.c_str(), 1, 30) ||
            IfInvaild(password.c_str(), 1, 30) ||
            IfInvaild(password.c_str(), 2, 30))
            throw Error();
        if (user_stack.empty()) throw Error();
        if (user_stack.rbegin()->first.GetPriority() < 3) throw Error();
        int priorityy = atoi(priority.c_str());
        if ((priorityy != 1 && priorityy != 3) ||
            priorityy >= user_stack.rbegin()->first.GetPriority())
            throw Error();
        user_stack.rbegin()->first.UserAdd(user_id, password, priorityy,
                                           user_name);
        log_all.Record(user_stack.rbegin()->first.GetUserId() + " " +
                       std::to_string(user_stack.rbegin()->first.GetPriority()) +
                       " add " + user_id);
        return "1";
    } else if (token == "delete") {
        if (user_stack.empty() || user_stack.rbegin()->first.GetPriority() < 7)
            throw Error();
        std::string user_id;
        user_id = line.nextToken();
        if (line.nextToken() != kMod) throw Error();
        for (auto iter : user_stack)
            if (iter.first.GetUserId() == user_id) throw Error();
        Delete(user_id);  //
        log_all.Record(user_stack.rbegin()->first.GetUserId() + " " +
                       std::to_string(user_stack.rbegin()->first.GetPriority()) +
                       " delete " + user_id);
        return "1";
    } else if (token == "show") {  ////////
        if (user_stack.empty()) throw Error();
        if (user_stack.rbegin()->first.GetPriority() == 0) throw Error();
        std::string show_ans(Show(user_stack.rbegin()->first, line,
                             user_stack.rbegin()->first.GetPriority())
                            .str());
        log_all.Record(user_stack.rbegin()->first.GetUserId() + " " +
                       std::to_string(user_stack.rbegin()->first.GetPriority()) +
                       " show");
        return show_ans;
    } else if (token == "buy") {
        if (user_stack.empty()) throw Error();
        if (user_stack.rbegin()->first.GetPriority() == 0) throw Error();
        BuyBook(user_stack.rbegin()->first, line);
        return "1";
    } else if (token == "select") {  ////////
        if (user_stack.empty()) throw Error();
        if (user_stack.rbegin()->first.GetPriority() < 3) throw Error();
        // operations to be modified
        user_stack.rbegin()->second = Select(user_stack.rbegin()->first, line);
        log_all.Record(user_stack.rbegin()->first.GetUserId() + " " +
                       std::to_string(user_stack.rbegin()->first.GetPriority()) +
                       " select");
        return "1";
    } else if (token == "modify") {
        if (user_stack.empty()) throw Error();
        if (user_stack.rbegin()->second.GetISBN() == "") throw Error();
        if (user_stack.rbegin()->first.GetPriority() < 3) throw Error();
        ModifyBook(user_stack.rbegin()->first, user_stack.rbegin()->second,
                   line);
        return "1";
    } else if (token == "import") {
        if (user_stack.empty()) throw Error();
        if (user_stack.rbegin()->second.GetISBN() == "") throw Error();
        if (user_stack.rbegin()->first.GetPriority() < 3) throw Error();
        std::string quantity = line.nextToken(), total_cost = line.nextToken();
        if (line.nextToken() != kMod || quantity == kMod || total_cost == kMod)
            throw Error();
        if (IfInvaild(quantity.c_str(), 3, 10) ||
            IfInvaild(total_cost.c_str(), 5, 13))
            throw Error();
        if (quantity.length() == 10 && quantity > "2147483647") throw Error();
        Import(user_stack.rbegin()->first, user_stack.rbegin()->second,
               atoi(quantity.c_str()), atof(total_cost.c_str()));
        Log log("log");
        log.Record("-" + total_cost);
        return "1";
    } else if (token == "report") {
        if (user_stack.empty()) throw Error();
        token = line.nextToken();
        if (line.nextToken() != kMod) throw Error();
        if (token == "myself") {
            if (user_stack.rbegin()->first.GetPriority() >= 3)
                log_all.ShowMyself(user_stack.rbegin()->first);
            else
                throw Error();
        } else if (token == "employee") {
            if (user_stack.rbegin()->first.GetPriority() == 7)
                log_all.ShowEmployee();
            else
                throw Error();
        } else if (token == "finance") {
            if (user_stack.rbegin()->first.GetPriority() == 7)
                log_all.ShowTransaction();
            else
                throw Error();
        } else
            throw Error();
        return "1";
    } else if (token == "log") {
        if (user_stack.empty()) throw Error();
        if (user_stack.rbegin()->first.GetPriority() == 7)
            log_all.Show();
        else
            throw Error();
        return "1";
    } else
        throw Error();
}

void RunFrontEnd() {
    const int BUFFER_SIZE = 102400;
    int slisten = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(7777);
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(slisten, (struct sockaddr *)&sin, sizeof(sin)) == -1)
        printf("bind error!");
    if (listen(slisten, 5) == -1) {
        printf("listen error!");
        exit(0);
    }
    int sclient;
    sockaddr_in client_add;
    socklen_t naddrlen = sizeof(client_add);
    char revdata[BUFFER_SIZE], command[BUFFER_SIZE], send_result[BUFFER_SIZE];

    while (1) {
        memset(command, 0, sizeof(command));
        memset(revdata, 0, sizeof(revdata));
        sclient = accept(slisten, (struct sockaddr *)&client_add, &naddrlen);
        if (sclient == -1) {
            printf("Socket:Accept Error!");
            continue;
        }
        recv(sclient, revdata, BUFFER_SIZE, 0);
        std::cout << "recieve command: " << revdata << " length: " << strlen(revdata)
             << std::endl;
        std::string result_str = Process(revdata);
        send(sclient, result_str.c_str(), result_str.length(), 0);
    }
}

int main() {
    try {
        Account tmp("root", "", "sjtu", 7);
        account_info.WriteInfo(tmp, "root");
    } catch (Error &ex) {
    }
    RunFrontEnd();
    return 0;
}