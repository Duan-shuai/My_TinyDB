#pragma once
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <iostream>

#include "rwlatch.h"
namespace db
{
    class TrieNode
    {
    public:
        explicit TrieNode(char key_char)
        {
            key_char_ = key_char;
            is_end_ = false;
        }

        TrieNode(TrieNode &&other_trie_node) noexcept
        {
            key_char_ = other_trie_node.key_char_;
            is_end_ = other_trie_node.is_end_;
            children_ = std::move(other_trie_node.children_);
        }

        virtual ~TrieNode() = default;

        bool HasChild(char key_char) const
        {
            auto it = children_.find(key_char);
            return it != children_.cend();
        }

        bool HasChildren() const { return !children_.empty(); }

        bool IsEndNode() const { return is_end_; }

        char GetKeyChar() const { return key_char_; }

        std::unique_ptr<TrieNode> *InsertChildNode(char key_char, std::unique_ptr<TrieNode> &&child)
        {
            if (HasChild(key_char) || child->GetKeyChar() != key_char)
            {
                return nullptr;
            }
            children_[key_char] = std::move(child);
            return &children_[key_char];
        }

        std::unique_ptr<TrieNode> *GetChildNode(char key_char)
        {
            auto it = children_.find(key_char);
            if (it == children_.cend())
            {
                return nullptr;
            }
            return &it->second;
        }

        void RemoveChildNode(char key_char)
        {
            auto it = children_.find(key_char);
            if (it == children_.cend())
            {
                return;
            }
            children_.erase(it);
        }

        void SetEndNode(bool is_end) { is_end_ = is_end; }

    protected:
        char key_char_;

        bool is_end_{false};

        std::unordered_map<char, std::unique_ptr<TrieNode>> children_;
    };

    template <typename T>
    class TrieNodeWithValue : public TrieNode
    {
    private:
        T value_;

    public:
        TrieNodeWithValue(TrieNode &&trieNode, T value) : TrieNode(std::move(trieNode))
        {
            value_ = value;
            is_end_ = true;
        }

        TrieNodeWithValue(char key_char, T value) : TrieNode(key_char), value_(value) { is_end_ = true; }

        ~TrieNodeWithValue() override = default;

        T GetValue() const { return value_; }
    };

    class Trie
    {
    private:
        std::unique_ptr<TrieNode> root_;

        ReaderWriterLatch latch_;

    public:
        Trie() { root_ = std::make_unique<TrieNode>('\0'); }

        template <typename T>
        bool Insert(const std::string &key, T value)
        {
            latch_.WLock();
            auto length = key.length();
            if (length == 0)
            {
                latch_.WUnlock();
                return false;
            }
            auto current = &root_;
            decltype(length) now = 0;
            while (now < length - 1)
            {
                auto child = (*current)->GetChildNode(key[now]);
                if (child == nullptr)
                {
                    current->get()->InsertChildNode(key[now], std::make_unique<TrieNode>(TrieNode(key[now]))); // 生成子节点并插入
                    current = (*current)->GetChildNode(key[now]);
                }
                else
                {
                    current = child;
                }
                now++;
            }
            auto terminal = (*current)->GetChildNode(key[now]);
            if (terminal != nullptr)
            {

                if ((*terminal)->IsEndNode())
                {
                    latch_.WUnlock();
                    return false;
                }
                terminal->reset(new TrieNodeWithValue<T>(std::move(*(*terminal)), value));
                terminal->get()->SetEndNode(true);
            }
            else
            {
                current->get()->InsertChildNode(key[now], std::make_unique<TrieNodeWithValue<T>>(key[now], value));
                current->get()->GetChildNode(key[now])->get()->SetEndNode(true);
            }
            latch_.WUnlock();
            return true;
        }

        bool Remove(const std::string &key)
        {
            latch_.WLock();
            auto length = key.length();
            auto current = &root_;
            std::vector<decltype(current)> node_stack;
            if (length == 0)
            {
                latch_.WUnlock();
                return false;
            }
            decltype(length) now = 0;
            while (now < length - 1)
            {
                auto child = (*current)->GetChildNode(key[now]);
                if (child == nullptr)
                {
                    latch_.WUnlock();
                    return false;
                }
                node_stack.push_back(current);
                current = child;
                now++;
            }
            auto terminal = (*current)->GetChildNode(key[now]);
            if (terminal == nullptr)
            {
                latch_.WUnlock();
                return false;
            }
            terminal->get()->SetEndNode(false);
            node_stack.push_back(terminal);
            auto trie_end = node_stack.rbegin();
            while (trie_end != node_stack.crend())
            {
                auto current_node = *trie_end;
                if (current_node->get()->IsEndNode() || current->get()->HasChildren())
                {
                    break;
                }
                auto child_char = (*trie_end)->get()->GetKeyChar();
                (*(--trie_end))->get()->RemoveChildNode(child_char);
            }
            latch_.WUnlock();
            return true;
        }
        template <typename T>
        T GetValue(const std::string &key, bool *success)
        {

            latch_.WLock();
            *success = false;
            auto current = &root_;
            auto length = key.length();
            decltype(length) now = 0;
            if (length == 0)
            {
                *success = false;
                latch_.WUnlock();
                return {};
            }
            while (now < length)
            {
                auto child = (*current)->GetChildNode(key[now]);
                if (child == nullptr)
                {
                    *success = false;
                    break;
                }
                current = child;
                now++;
            }
            if (now == length)
            {
                if (!(*current)->IsEndNode())
                {
                    *success = false;
                }
                else
                {
                    auto terminal = dynamic_cast<TrieNodeWithValue<T> *>(current->get());
                    if (terminal == nullptr)
                    {
                        *success = false;
                    }
                    else
                    {
                        *success = true;
                        latch_.WUnlock();
                        return (*terminal).GetValue();
                    }
                }
            }
            else
            {
                *success = false;
            }
            latch_.WUnlock();
            return {};
        }
    };
}

