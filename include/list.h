#ifndef LIST_H
#define LIST_H

#include <types.h>

template<class T>
class List
{
public:
    class Node
    {
        friend class List;
        Node *Next;
    public:
        T Value;
        Node(Node *next, T value) : Next(next), Value(value) {}
        Node(T value) : Node(nullptr, value) {}
    };
    typedef bool (*Comparer)(T a, T b);
    static bool defaultComparer(T a, T b)
    {
        return a == b;
    }
private:
    Node *First;
    struct Iterator
    {
        Node *Current;
        T operator *() { return Current->Value; }
        Iterator operator ++()
        {
            Current = Current->Next;
            return { Current };
        }
        bool operator !=(Iterator b) { return this->Current != b.Current; }
    };
public:
    List() : First(nullptr)
    {
    }

    void Clear()
    {
        for(Node *n = First; n;)
        {
            Node *next = n->Next;
            delete n;
            n = next;
        }
        First = nullptr;
    }

    void Prepend(T value)
    {
        First = new Node(First, value);
    }

    void Append(T value)
    {
        if(!First)
            First = new Node(nullptr, value);
        else
        {
            Node *n = nullptr;
            for(n = First; n->Next; n = n->Next);
            n->Next = new Node(nullptr, value);
        }
    }

    uint Remove(T value, Comparer comparer, bool all)
    {
        if(!comparer) comparer = defaultComparer;
        uint removed = 0;
        for(Node *prev = nullptr, *node = First; node; prev = node, node = node->Next)
        {
            if(comparer(node->Value, value))
            {
                if(!prev) First = node->Next;
                else prev->Next = node->Next;
                delete node;
                ++removed;
                if(!all) return removed;
            }
        }
        return removed;
    }

    bool Contains(T value, Comparer comparer)
    {
        for(Node *prev = nullptr, *node = First; node; prev = node, node = node->Next)
        {
            if(comparer(node->Value, value))
                return true;
        }
        return false;
    }

    uint Count()
    {
        uint count = 0;
        for(Node *prev = nullptr, *node = First; node; prev = node, node = node->Next)
            ++count;
        return count;
    }

    uint Count(T value, Comparer comparer)
    {
        uint count = 0;
        for(Node *prev = nullptr, *node = First; node; prev = node, node = node->Next)
        {
            if(comparer(node->Value, value))
                ++count;
        }
        return count;
    }

    T Get(uint idx)
    {
        uint i = 0;
        for(Node *node = First; node; node = node->Next, ++i)
        {
            if(idx == i)
                return node->Value;
        }
        return T();
    }

    T operator[](uint idx)
    {
        return Get(idx);
    }

    T Find(T value, Comparer comparer)
    {
        if(!comparer) comparer = defaultComparer;
        for(Node *prev = nullptr, *node = First; node; prev = node, node = node->Next)
        {
            if(comparer(node->Value, value))
                return node->Value;
        }
        return nullptr;
    }

    Iterator begin()
    {
        return { First };
    }


    Iterator end()
    {
        return { nullptr };
    }

    ~List()
    {
        for(Node *n = First; n;)
        {
            Node *next = n->Next;
            delete n;
            n = next;
        }
    }
};

#endif // LIST_H
