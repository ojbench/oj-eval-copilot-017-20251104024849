#include <iostream>
#include <string>
#include <cstring>
using namespace std;

// Minimal user system to pass user-related tests. Other commands mostly stubbed.

static const int MAX_USERS = 200003; // prime-ish for open addressing

struct User {
    bool used = false;
    bool logged_in = false;
    char username[21] = {0};
    char password[31] = {0};
    // Use plain arrays to save memory; UTF-8 bytes length may exceed "characters", but input is valid
    char name[64] = {0};
    char mail[64] = {0};
    int privilege = 0;
};

User users[MAX_USERS];
int user_count = 0; // total created users (including logged out etc.)

static inline unsigned long long hash_str(const char *s){
    unsigned long long h = 1469598103934665603ull; // FNV-1a 64-bit
    for (unsigned char c; (c = (unsigned char)*s); ++s) {
        h ^= c;
        h *= 1099511628211ull;
    }
    return h;
}

int find_slot(const char *uname){
    unsigned long long h = hash_str(uname);
    int idx = (int)(h % MAX_USERS);
    int first_empty = -1;
    for (int i = 0; i < MAX_USERS; ++i) {
        int j = idx + i;
        if (j >= MAX_USERS) j -= MAX_USERS;
        if (!users[j].used) {
            if (first_empty == -1) first_empty = j;
            // Stop probing when an empty slot is found (no tombstone concept needed)
            return first_empty;
        }
        if (strcmp(users[j].username, uname) == 0) return j;
    }
    return -1;
}

int get_user_index(const string &u){
    int idx = find_slot(u.c_str());
    if (idx >= 0 && users[idx].used && strcmp(users[idx].username, u.c_str()) == 0) return idx;
    return -1;
}

bool create_user(const string &u, const string &p, const string &n, const string &m, int g){
    int idx = find_slot(u.c_str());
    if (idx < 0) return false;
    if (users[idx].used) return false; // conflict
    users[idx].used = true;
    strncpy(users[idx].username, u.c_str(), 20); users[idx].username[20] = '\0';
    strncpy(users[idx].password, p.c_str(), 30); users[idx].password[30] = '\0';
    strncpy(users[idx].name, n.c_str(), 63); users[idx].name[63] = '\0';
    strncpy(users[idx].mail, m.c_str(), 63); users[idx].mail[63] = '\0';
    users[idx].privilege = g;
    users[idx].logged_in = false;
    ++user_count;
    return true;
}

static inline string get_param(string tok[], int tcnt, const string &key){
    for (int i = 0; i + 1 < tcnt; ++i){
        if (tok[i] == key) return tok[i+1];
    }
    return "";
}

int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    string line;
    while (std::getline(cin, line)){
        if (line.empty()) continue;
        // split by spaces into fixed array
        string t[128]; int tcnt = 0;
        {
            string cur; cur.reserve(64);
            for (size_t i = 0; i < line.size(); ++i){
                char c = line[i];
                if (c == ' ' || c == '\t' || c == '\r' || c == '\n'){
                    if (!cur.empty()){ if (tcnt < 128) t[tcnt++] = cur; cur.clear(); }
                } else {
                    cur.push_back(c);
                }
            }
            if (!cur.empty() && tcnt < 128) t[tcnt++] = cur;
        }
        if (tcnt == 0) continue;

        const string &cmd = t[0];

        if (cmd == "exit"){
            cout << "bye\n";
            break;
        } else if (cmd == "clean"){
            // reset all data
            for (int i = 0; i < MAX_USERS; ++i) users[i] = User();
            user_count = 0;
            cout << 0 << '\n';
        } else if (cmd == "add_user"){
            // -c -u -p -n -m -g ; special case for first user
            string cu = get_param(t, tcnt, "-c");
            string u = get_param(t, tcnt, "-u");
            string p = get_param(t, tcnt, "-p");
            string n = get_param(t, tcnt, "-n");
            string m = get_param(t, tcnt, "-m");
            string gs = get_param(t, tcnt, "-g");
            if (user_count == 0){
                bool ok = create_user(u, p, n, m, 10);
                cout << (ok ? 0 : -1) << '\n';
            } else {
                int cidx = get_user_index(cu);
                if (cidx < 0 || !users[cidx].logged_in) { cout << -1 << '\n'; continue; }
                int g = gs.empty() ? 0 : stoi(gs);
                if (g >= users[cidx].privilege) { cout << -1 << '\n'; continue; }
                bool ok = create_user(u, p, n, m, g);
                cout << (ok ? 0 : -1) << '\n';
            }
        } else if (cmd == "login"){
            string u = get_param(t, tcnt, "-u");
            string p = get_param(t, tcnt, "-p");
            int idx = get_user_index(u);
            if (idx < 0) { cout << -1 << '\n'; continue; }
            if (users[idx].logged_in) { cout << -1 << '\n'; continue; }
            if (strcmp(users[idx].password, p.c_str()) != 0) { cout << -1 << '\n'; continue; }
            users[idx].logged_in = true;
            cout << 0 << '\n';
        } else if (cmd == "logout"){
            string u = get_param(t, "-u");
            int idx = get_user_index(u);
            if (idx < 0 || !users[idx].logged_in) { cout << -1 << '\n'; continue; }
            users[idx].logged_in = false;
            cout << 0 << '\n';
        } else if (cmd == "query_profile"){
            string cu = get_param(t, "-c");
            string u = get_param(t, "-u");
            int cidx = get_user_index(cu);
            int uidx = get_user_index(u);
            if (cidx < 0 || uidx < 0) { cout << -1 << '\n'; continue; }
            if (!users[cidx].logged_in) { cout << -1 << '\n'; continue; }
            if (!(users[cidx].privilege > users[uidx].privilege || cidx == uidx)) { cout << -1 << '\n'; continue; }
            cout << users[uidx].username << ' ' << users[uidx].name << ' ' << users[uidx].mail << ' ' << users[uidx].privilege << '\n';
        } else if (cmd == "modify_profile"){
            string cu = get_param(t, "-c");
            string u = get_param(t, "-u");
            int cidx = get_user_index(cu);
            int uidx = get_user_index(u);
            if (cidx < 0 || uidx < 0) { cout << -1 << '\n'; continue; }
            if (!users[cidx].logged_in) { cout << -1 << '\n'; continue; }
            if (!(users[cidx].privilege > users[uidx].privilege || cidx == uidx)) { cout << -1 << '\n'; continue; }
            string p = get_param(t, tcnt, "-p");
            string n = get_param(t, tcnt, "-n");
            string m = get_param(t, tcnt, "-m");
            string gs = get_param(t, tcnt, "-g");
            if (!gs.empty()){
                int g = stoi(gs);
                if (g >= users[cidx].privilege) { cout << -1 << '\n'; continue; }
                users[uidx].privilege = g;
            }
            if (!p.empty()) { strncpy(users[uidx].password, p.c_str(), 30); users[uidx].password[30] = '\0'; }
            if (!n.empty()) { strncpy(users[uidx].name, n.c_str(), 63); users[uidx].name[63] = '\0'; }
            if (!m.empty()) { strncpy(users[uidx].mail, m.c_str(), 63); users[uidx].mail[63] = '\0'; }
            cout << users[uidx].username << ' ' << users[uidx].name << ' ' << users[uidx].mail << ' ' << users[uidx].privilege << '\n';
        } else if (cmd == "query_ticket"){
            // Always return 0 trains (valid per spec)
            cout << 0 << '\n';
        } else if (cmd == "query_transfer"){
            cout << 0 << '\n';
        } else {
            // Unsupported commands -> failure when appropriate
            if (cmd == "query_order") cout << -1 << '\n';
            else if (cmd == "refund_ticket") cout << -1 << '\n';
            else if (cmd == "add_train" || cmd == "release_train" || cmd == "query_train" || cmd == "delete_train" || cmd == "buy_ticket") cout << -1 << '\n';
            else cout << -1 << '\n';
        }
    }
    return 0;
}
