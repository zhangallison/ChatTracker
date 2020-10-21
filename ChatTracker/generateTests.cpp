/*#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <random>
#include <algorithm>
#include <utility>
#include <cmath>
#include <cassert>
using namespace std;

const int NCHATS = 1000;
const int NUSERS = 10000;

random_device rd;
default_random_engine gen(rd());

inline int randInt(int lo, int hi)
{
    return uniform_int_distribution<int>(lo, hi)(gen);
}

template<typename Cont, typename T>
inline bool has(const Cont& cont, T value)
{
    return find(cont.begin(), cont.end(), value) != cont.end();
}

struct Chat
{
    vector<int> users;
};

struct ChatInfo
{
    ChatInfo(int c)
     : chat(c), in(false), prev(-1), contribs(randInt(1, 3))
    {}
    bool operator==(int c) const { return chat == c; }
    int chat;
    bool in;
    int prev;
    int contribs;
};

struct User
{
    User() : curr(-1) {}
    vector<ChatInfo> chatinfos;
    int curr;
};

void remove(User& u, int c)
{
    if (u.curr == c)
        u.curr = u.chatinfos[c].prev;
    else
    {
        for (int k = 0; k < u.chatinfos.size(); k++)
        {
            if (u.chatinfos[k].prev == c)
            {
                u.chatinfos[k].prev = u.chatinfos[c].prev;
                break;
            }
        }
    }
    u.chatinfos[c].in = false;
    u.chatinfos[c].prev = -1;
}

void addEvent(vector<int>& actives, int u)
{
    actives.push_back(u);
    swap(actives.back(), actives[randInt(0, actives.size() - 1)]);
}

ofstream outf;

string genName(char c, int n)
{
    ostringstream oss;
    oss << string(11, c) << setw(5) << setfill('0') << n;
    return oss.str();
}

void genJoin(int u, int c)
{
    outf << "j " << genName('u', u) << " " << genName('c', c) << endl;
}

void genTerminate(int c)
{
    outf << "t " << genName('c', c) << endl;
}

void genContribute(int u)
{
    outf << "c " << genName('u', u) << endl;
}

void genLeave(int u, int c)
{
    outf << "l " << genName('u', u) << " " << genName('c', c) << endl;
}

void genLeave(int u)
{
    outf << "l " << genName('u', u) << endl;
}

int main()
{
    cout << "Enter output file name: ";
    string filename;
    getline(cin,filename);
    outf.open(filename);
    if (!outf)
    {
        cout << "Cannot create " << filename << endl;
        return 1;
    }

    auto distroChatChosenByUser = []
      {
        vector<double> weights;
        int k;
        for (k = 0; k < NCHATS; k++)
            weights.push_back(1.0 / pow(k+2, 1.2));
        return discrete_distribution<int>(weights.begin(), weights.end());
      }();
    auto distroNumChatsPerUser = []
      {
        vector<double> weights;
        weights.push_back(0);  // no chance of 0 chats
        int k;
        for (k = 1; k < 35; k++)
        {
            double a = (k - 2) / 2;
            weights.push_back(exp(-0.5 * a * a));
        }
        double minwt = weights.back();
        for ( ; k < 50; k++)
            weights.push_back(minwt);
        return discrete_distribution<int>(weights.begin(), weights.end());
      }();

    vector<Chat> chats(NCHATS);
    vector<User> users(NUSERS);;
    for (int u = 0; u < NUSERS; u++)
    {
        int nchats = distroNumChatsPerUser(gen);
        for (int k = 0; k < nchats; k++)
        {
            int c = distroChatChosenByUser(gen);
            if ( ! has(chats[c].users, u))
            {
                chats[c].users.push_back(u);
                users[u].chatinfos.push_back(ChatInfo(c));
            }
        }
    }

    vector<int> actives;
    for (int u = 0; u < users.size(); u++)
    {
        for (const auto& ci : users[u].chatinfos)
        {
            for (int k = 0; k < ci.contribs; k++)
                actives.push_back(u);
        }
    }
    shuffle(actives.begin(), actives.end(), gen);

    while ( ! actives.empty())
    {
        int u = actives.back();
        actives.pop_back();
        int r = randInt(1, 2000);
        if (r == 1)
        {
            int c = randInt(0, chats.size() - 1);
            for (int uu : chats[c].users)
            {
                vector<ChatInfo>& cis = users[uu].chatinfos;
                int k;
                for (k = 0; k < cis.size(); k++)
                {
                    if (cis[k].chat == c)
                        break;
                }
                if (cis[k].in)
                    remove(users[uu], k);
            }
            genTerminate(c);
            addEvent(actives, u);
            continue;
        }
        else if (r == 2)
        {
            int c = randInt(0, chats.size() - 1);
            if ( ! has(users[u].chatinfos, c))
            {
                genLeave(u, c);
                addEvent(actives, u);
                continue;
            }
        }
        if (users[u].curr == -1)
        {
            int r = randInt(1, 1000);
            if (r == 1)
            {
                genContribute(u);
                addEvent(actives, u);
                continue;
            }
            else if (r == 2)
            {
                genLeave(u);
                addEvent(actives, u);
                continue;
            }

            int c = randInt(0, users[u].chatinfos.size() - 1);
            ChatInfo& ci = users[u].chatinfos[c];
            ci.in = true;
            ci.prev = users[u].curr;
            users[u].curr = c;
            genJoin(u, users[u].chatinfos[c].chat);
            addEvent(actives, u);
            continue;
        }
        r = randInt(1, 1000);
        if (r == 1)
        {
            genJoin(u, users[u].chatinfos[users[u].curr].chat);
            addEvent(actives, u);
            continue;
        }
        else if (r <= 5)
        {
            if (randInt(1, 4) == 1)
            {
                int c = randInt(0, users[u].chatinfos.size() - 1);
                while ( ! users[u].chatinfos[c].in)
                {
                    c++;
                    if (c == users[u].chatinfos.size())
                        c = 0;
                }
                remove(users[u], c);
                genLeave(u, users[u].chatinfos[c].chat);
            }
            else
            {
                remove(users[u], users[u].curr);
                genLeave(u);
            }
            addEvent(actives, u);
            continue;
        }
        if (randInt(1, 3) == 1)
        {
            int c = randInt(0, users[u].chatinfos.size() - 1);
            ChatInfo& ci = users[u].chatinfos[c];
            if (ci.in)
                remove(users[u], c);
            ci.in = true;
            ci.prev = users[u].curr;
            users[u].curr = c;
            genJoin(u, users[u].chatinfos[c].chat);
            addEvent(actives, u);
            continue;
        }
        ChatInfo& ci = users[u].chatinfos[users[u].curr];
        if (ci.contribs > 0)
        {
            ci.contribs--;
            genContribute(u);
            continue;
        }
    }
}
*/
