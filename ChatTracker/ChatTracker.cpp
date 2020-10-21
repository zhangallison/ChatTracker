
#include "ChatTracker.h"
#include <string>
#include <list>
#include <vector>
#include <functional>
#include <utility>
using namespace std;


// Templated HashMap class declaration
// Class accepts two different types of data types: one that represents the key value and one that represents the value
template <typename KeyType, typename ValueType>
class HashMap
{
public:
    HashMap(int maxBuckets);
    ~HashMap();
    void associate(const KeyType& key, const ValueType& value);
    void erase(const KeyType& key);
    ValueType* find(const KeyType& key);
private:
    vector<list<pair<KeyType, ValueType>>> m_map;
    int m_maxBucks;
    size_t m_size;
    unsigned getBucketNumber(const KeyType& key)
    {
        std::hash<KeyType> hash;
        unsigned h = hash(key);
        return h % m_map.size();
    }
};


// User class declaration
// Each user object has a list of Chat struct objects that keeps track of the user's contributinos to that chat
class User
{
public:
    User(string name, int maxBuckets);
    ~User();
    bool operator==(const User& other);
    string name();
    int currentCount();
    void addCurrentChat(const string& name);
    bool getCurrentChat(string& name);
    int leaveChat(const string& name);
    int leaveCurrentChat();
    void setCurrentCount(int num);
    
private:
    string m_name;
    struct Chat
    {
        string name;
        int count;
    };
    list<Chat> m_allChats;
    int m_numBucks;
};
    
class ChatTrackerImpl
{
public:
    ChatTrackerImpl(int maxBuckets);
    void join(string user, string chat);
    int terminate(string chat);
    int contribute(string user);
    int leave(string user, string chat);
    int leave(string user);
    
private:
    // Hash table that hashes by user's name and returns a User object:
    HashMap<string, User> m_users;
    // Hash table that hashes by chat's name and returns an integer that tracks number of contributions to that chat:
    HashMap<string, int> m_chatCount;
    // Hash table that hashes by chat's name and returns a list of strings that represents the names of users in the chat:
    HashMap<string, list<string>> m_chatID;
    
    int m_numBucks;
};

template<typename KeyType, typename ValueType>
HashMap<KeyType, ValueType>::HashMap(int maxBuckets) : m_map(maxBuckets)
{
    m_size = 0;
    m_maxBucks = maxBuckets;
}

template<typename KeyType, typename ValueType>
HashMap<KeyType, ValueType>::~HashMap()
{
    
}


template<typename KeyType, typename ValueType>
void HashMap<KeyType, ValueType>::associate(const KeyType& key, const ValueType& value)
{
    // Determine bucket number by calling hash function on key
    unsigned bucketNum = getBucketNumber(key);
    
    // Look for value in the list of pairs at the bucketNumber
    for(pair<KeyType, ValueType>& p : m_map[bucketNum])
    {
        // If the key in the pair in the linked list matches the key we want, set value to the matching value and return
        if(p.first == key)
        {
            p.second = value;
            return;
        }
    }
    
    // if key is not already in map:
    // Insert the key-value pair into the linked list at the determined bucket number
    m_map[bucketNum].emplace_back(key, value);
    m_size++;
}

template<typename KeyType, typename ValueType>
void HashMap<KeyType, ValueType>::erase(const KeyType& key)
{
    // Determine the bucket number where the key should be located by calling hash function on key
    unsigned bucketNum = getBucketNumber(key);
    
    // Find the value in hashmap
    ValueType* v = find(key);
    // If value is not found, nothing to erase, return
    if(v == nullptr)
        return;
    
    // Create a pair with the same values as the key and the found value and remove it from the list
    pair<KeyType, ValueType> p;
    p.first = key;
    p.second = *v;
    m_map[bucketNum].remove(p);
    m_size--;
}

template<typename KeyType, typename ValueType>
ValueType* HashMap<KeyType, ValueType>::find(const KeyType &key)
{
    // Determine the bucket number where the key should be located by calling hash function on key
    unsigned bucketNum = getBucketNumber(key);
    
    // Look for value in the list of pairs at the bucketNumber
    for(pair<KeyType, ValueType>& p : m_map[bucketNum])
    {
        // If the key in the pair in the linked list matches the key we want, return address of matching value
        if(p.first == key)
        {
            return &p.second;
        }
    }
    return nullptr;
}


// *************** User implementations *******************
User::User(string name, int maxBuckets)
{
    m_numBucks = maxBuckets;
    m_name = name;
}

User::~User()
{
    
}

bool User::operator==(const User& other)
{
    if(m_name == other.m_name)
    {
        return true;
    }
    return false;
}

string User::name()
{
    return m_name;
}

int User::currentCount()
{
    // Return the user's current count by looking at the count of the front most chat of the list
    if(!m_allChats.empty())
    {
        Chat c = m_allChats.front();
        return c.count;
    }
    return 0;
}

void User::addCurrentChat(const string& name)
{
    list<Chat>::iterator it;
    Chat oldChat;
    bool exists = false;
    // Look through all of the user's existing chats to see if user is already associated with chat
    for(it = m_allChats.begin(); it != m_allChats.end(); it++)
    {
        // If user is already associated with chat (i.e. chat is already within user's list of chats)
        if(it->name == name)
        {
            // Create new chat object with same variables as the chat found
            oldChat.name = it->name;
            oldChat.count = it->count;
            // Erase the chat found
            it = m_allChats.erase(it);
            exists = true;
            break;
        }
    }
    if(exists)
    {
        // If the user was already associated with the chat, push the newly created chat variable to the front of the list
        m_allChats.push_front(oldChat);
    }
    else
    {
        // Otherwise if the user was not associated with the chat
        // Create a new chat variable and push it to the front of the user's list of chats
        int count = 0;
        Chat c;
        c.name = name;
        c.count = count;
        m_allChats.push_front(c);
    }
    
}

// Purpose: store the name of the current chat into the passed string parameter
// Returns true if user has a current chat (i.e. user's list is not empty)
// Returns false if user does not have a current chat (i.e. user's list is empty)
bool User::getCurrentChat(string& name)
{
    if(!m_allChats.empty())
    {
        // Return the name of the user's current chat by looking at the front of the users' list of chats
        name = m_allChats.front().name;
        return true;
    }
    return false;
}

// Purpose: remove chat from user's list of chats and store the user's contributions in that chat in the count variable
// Returns true if user is associated with chat
// Returns false if user is not associated with chat
int User::leaveChat(const string& name)
{
    int result = -1;
    
    if(!m_allChats.empty())
    {
        // Look through user's list of Chat struct objects
        list<Chat>::iterator it;
        for(it = m_allChats.begin(); it != m_allChats.end(); it++)
        {
            // If current Chat object's name matches name passed in
            if(it->name == name)
            {
                // Store the chat's associated number of contributions
                result = it->count;
                // Erase the chat from the list from the user's list of chat objects
                it = m_allChats.erase(it);
                break;
            }
        }
    }
    return result;
}

// Purpose: remove the front chat from the user's list of chats
//          and store the user's contributions in the count variable passed in
// Returns true if user has a current chat
// Returns false if user does not have a current chat
int User::leaveCurrentChat()
{
    int result = -1;
    if(!m_allChats.empty())
    {
        result = m_allChats.front().count;
        m_allChats.erase(m_allChats.begin());
    }
    return result;
}

void User::setCurrentCount(int num)
{
    // Sets the contributions of the front chat equal to num
    if(!m_allChats.empty())
    {
        m_allChats.front().count = num;
    }
}

// *************** ChatTrackerImpl implementations *******************

ChatTrackerImpl::ChatTrackerImpl(int maxBuckets) : m_users(maxBuckets), m_chatCount(maxBuckets), m_chatID(maxBuckets), m_numBucks(maxBuckets)
{
    
}

void ChatTrackerImpl::join(string user, string chat)
{
    User* u = m_users.find(user);
    
    // User exists: add chat or make chat their current chat
    if(u != nullptr)
    {
        // Call the user's add current chat method
        u->addCurrentChat(chat);
        
        // Find chat in hash table to add user to that chat's list of strings
        list<string>* chatUsers = m_chatID.find(chat);
        
        // Chat exists: add user to list of strings
        if(chatUsers != nullptr)
        {
            chatUsers->push_back(user);
        }
        // Chat does not exist: add chat to hash table to chats and push user to the new chat's list of strings
        else
        {
            list<string> newChatUsers;
            newChatUsers.push_back(user);
            m_chatID.associate(chat, newChatUsers);
        }
    }
    // User does not exist: make a new user, add chat
    else
    {
        // Create a new user and add it to the hash table of users
        User newUser(user, m_numBucks);
        m_users.associate(user, newUser);
        m_users.find(user)->addCurrentChat(chat);
        
        // Find chat in hash table of chats to add user to that chat's list of strings
        list<string>* chatUsers = m_chatID.find(chat);
        
        // Chat exists: add new user to list of strings for chat
        if(chatUsers != nullptr)
        {
            chatUsers->push_back(user);
        }
        
        // Chat does not exist: add chat to hash table of chats and push user to new chat's list of strings
        else
        {
            list<string> newChatUsers;
            newChatUsers.push_back(user);
            m_chatID.associate(chat, newChatUsers);
        }
    }
    
    // See if the chat exists already in the hash table keeping track of chat's contributions
    // If it does exist, do nothing
    if(m_chatCount.find(chat) != nullptr)
        return;
    // Otherwise insert it into the hash table by associating chat table with a value of 0
    m_chatCount.associate(chat, 0);
    
    
}

int ChatTrackerImpl::terminate(string chat)
{
    // Find chat in hash table of chats
    list<string>* chatUsers = m_chatID.find(chat);
    if(chatUsers != nullptr)
    {
        // Iterate through the chat's list of strings (its list of users) and call leave chat on every user
        list<string>::iterator it;
        for(it = chatUsers->begin(); it != chatUsers->end(); it++)
        {
            User* u = m_users.find(*it);
            if(u != nullptr)
            {
                 u->leaveChat(chat);
            }
        }
    }
    // Erase the chat from the hash table of chats
    m_chatID.erase(chat);
    
    int count = 0;
    // Find the chat's name in the hash table of chats with their counts of contributions
    int* countPtr = m_chatCount.find(chat);
    if(countPtr != nullptr)
    {
        // store the value found in the hash table
        count = *countPtr;
    }
    // Erase the chat from the the hash table of chats with their counts of contributions
    m_chatCount.erase(chat);
    return count;
    
}

int ChatTrackerImpl::contribute(string user)
{
    // Find the user in the hash table of users
    User* u = m_users.find(user);
    // If the user exists:
    if(u != nullptr)
    {
        // Increment its contributions in its current chat
        u->setCurrentCount(u->currentCount()+1);
        
        string ch = "";
        // Get the name of the user's chat (if it has one)
        if(u->getCurrentChat(ch))
        {
            // Find name of chat in hash table of chats with chat's contributions
            int* count = m_chatCount.find(ch);
            if(count != nullptr)
            {
                // Increment the value found in the hash table so chat now has 1 greater contribution than it did before
                m_chatCount.associate(ch, (*count)+1);
            }
        }
        // Return the user's new contributions in its current chat (0 if there is no current chat)
        return u->currentCount();
    }
    // Or return 0 if the user does not exist
    return 0;
}

int ChatTrackerImpl::leave(string user, string chat)
{
    int contri = -1;
    // Find the user in the hash table of users
    User* u = m_users.find(user);
    
    // If the user exists:
    if(u != nullptr)
    {
        // Call leave chat on the user and store its amount of contributions in variable
        contri = u->leaveChat(chat);
        
            // Find chat in hash table of chats
            list<string>* chatUsers = m_chatID.find(chat);
            if(chatUsers != nullptr)
            {
                // Remove the user's name from the chat's list of users
                chatUsers->remove(user);
            }
    }
    
    // Return the variable that stored user's contributions (or -1 if user does not exist)
    return contri;
}

int ChatTrackerImpl::leave(string user)
{
    int contri = -1;
    // Find user in hash table of users:
    User* u = m_users.find(user);
    
    // If the user exists:
    if(u != nullptr)
    {
        string ch = "";
        // Get the user's current chat name (if it exists)
        if(u->getCurrentChat(ch))
        {
            // Call leave user and store user's contributions in variable
            contri = u->leaveCurrentChat();
            
            
            // Find chat in hash table of chats
            list<string>* chatUsers = m_chatID.find(ch);
            if(chatUsers != nullptr)
            {
                // Remove the user's name from list of chat's users
                chatUsers->remove(user);
            }
            
        }
    }
    // Return the variable that stored the user's contributions (or -1 if the user does not exist)
    return contri;
}

//*********** ChatTracker functions **************

// These functions simply delegate to ChatTrackerImpl's functions.

ChatTracker::ChatTracker(int maxBuckets)
{
    m_impl = new ChatTrackerImpl(maxBuckets);
}

ChatTracker::~ChatTracker()
{
    delete m_impl;
}

void ChatTracker::join(string user, string chat)
{
    m_impl->join(user, chat);
}

int ChatTracker::terminate(string chat)
{
    return m_impl->terminate(chat);
}

int ChatTracker::contribute(string user)
{
    return m_impl->contribute(user);
}

int ChatTracker::leave(string user, string chat)
{
    return m_impl->leave(user, chat);
}

int ChatTracker::leave(string user)
{
    return m_impl->leave(user);
}
