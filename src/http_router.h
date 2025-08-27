/**
 * @brief Http router Define
 * @file http_router.h
 * @copyright Licensed under the Apache License, Version 2.0
 */

#pragma once
#include <string>
#include <unordered_map>
#include "http_common.h"

namespace http
{
namespace server
{
template <typename T>
class HttpRouter;

template <typename T>
class HttpPathNode
{
public:
    explicit HttpPathNode(const std::string& path, T* data)
        : path_(path)
        , data_(data){};

    ~HttpPathNode()
    {
        for (auto pair : children_)
        {
            delete pair.second;
        }
        children_.clear();
    };

    HttpPathNode(const HttpPathNode&) = delete;
    HttpPathNode& operator=(const HttpPathNode&) = delete;

    template <typename U>
    friend class HttpRouter;

private:
    std::string path_;
    T* data_;
    std::unordered_map<std::string, HttpPathNode*> children_;
};

template <typename T>
class HttpRouter
{
public:
public:
    HttpRouter()
        : root_(new HttpPathNode<T>("", nullptr)){};

    ~HttpRouter()
    {
        if (root_ != nullptr)
        {
            delete root_;
            root_ = nullptr;
        }
        LOG_LOGGER_INFO("http server router destroyed");
    };

    HttpRouter(const HttpRouter&) = delete;
    HttpRouter& operator=(const HttpRouter&) = delete;

    void insert(const std::string& path, T* data)
    {
        if (data == nullptr)
        {
            throw std::runtime_error("data should be not empty");
        }

        if (path[0] != '/')
        {
            throw std::runtime_error("path should start with '/'");
        }

        if (path.find("..") != std::string::npos)
        {
            throw std::runtime_error("path is invalid");
        }

        urls::segments_view segments(path);
        if (segments.size() == 0)
        {
            root_->data_ = data;
            return;
        }

        auto segments_size = segments.size();

        auto next_node = root_;
        std::string key;
        for (auto iter = segments.begin(); iter != segments.end(); iter++)
        {
            key = std::string(*iter);
            if (key == "")
            {
                break;
            }

            auto children_iter = next_node->children_.find(key);
            if (children_iter != next_node->children_.end())
            {
                next_node = children_iter->second;
            }
            else
            {
                auto new_node = new HttpPathNode<T>(std::string(*iter), nullptr);
                next_node->children_[key] = new_node;
                next_node = new_node;
            }
        }
        next_node->data_ = data;
    };

    T* search(urls::segments_view segments)
    {
        if (segments.size() == 0)
        {
            // path is "/"
            return root_->data_;
        }

        auto current_node = root_;
        std::string key;
        for (auto iter = segments.begin(); iter != segments.end(); iter++)
        {
            key = std::string(*iter);
            if (key == "")
            {
                break;
            }

            auto children_iter = current_node->children_.find(key);
            if (children_iter == current_node->children_.end())
            {
                break;
            }
            current_node = children_iter->second;
        }

        if (current_node == root_)
        {
            return nullptr;
        }
        else
        {
            return current_node->data_;
        }
    }

    T* search(beast::string_view path)
    {
        if (path.size() == 0)
        {
            // path is empty
            return root_->data_;
        }

        if (path[0] != '/')
        {
            // path is invalid
            return nullptr;
        }

        urls::segments_view segments(path);
        if (segments.size() == 0)
        {
            // path is "/"
            return root_->data_;
        }

        auto current_node = root_;
        std::string key;
        for (auto iter = segments.begin(); iter != segments.end(); iter++)
        {
            key = std::string(*iter);
            if (key == "")
            {
                break;
            }

            auto children_iter = current_node->children_.find(key);
            if (children_iter == current_node->children_.end())
            {
                break;
            }
            current_node = children_iter->second;
        }

        if (current_node == root_)
        {
            return nullptr;
        }
        else
        {
            return current_node->data_;
        }
    };

private:
    HttpPathNode<T>* root_;
};

}  // namespace server
}  // namespace http