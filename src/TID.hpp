#pragma once

#include <map>
#include <string>
#include <vector>

class TID {
public:
    ~TID() {
        delete TID;
    }


    void CreateScope();

    void ExitScope();

    void CreateVariable(const std::wstring &);

    bool CheckVariable(const std::wstring &);

private:
    struct Node {
        Node(Node* parent = nullptr) {
            this->parent = parent;
        }

        ~Node() {
            for (size_t i = 0; i < scopes.size(); ++i) {
                delete scopes[i];
            }
        }

        std::map<std::wstring, int> variables;

        std::vector<Node*> scopes;
        Node* parent;
    };

    Node* TID;
    Node* current = TID;
};
