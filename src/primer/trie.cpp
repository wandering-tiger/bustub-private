#include "primer/trie.h"
#include <string_view>
#include "common/exception.h"

namespace bustub {

template <class T>
auto Trie::Get(std::string_view key) const -> const T * {
  //throw NotImplementedException("Trie::Get is not implemented.");

  // You should walk through the trie to find the node corresponding to the key. If the node doesn't exist, return
  // nullptr. After you find the node, you should use `dynamic_cast` to cast it to `const TrieNodeWithValue<T> *`. If
  // dynamic_cast returns `nullptr`, it means the type of the value is mismatched, and you should return nullptr.
  // Otherwise, return the value.
  std::shared_ptr<const TrieNode> trieNode= this->root_;
  if(trieNode==nullptr){
    return nullptr;
  }
  for(char c : key){
    auto temp=trieNode->children_.find(c);  //this is a bug that hard to find, I worked for several days
    if(temp == trieNode->children_.end()){
      return nullptr;
    }else{
      trieNode = temp -> second;  //const map can't get the key, "at" should be used
    }
  }
  if (!trieNode->is_value_node_) {
    return nullptr;
  }
  const TrieNodeWithValue<T>* NodeWithValue = dynamic_cast<const TrieNodeWithValue<T> *>(trieNode.get());
  if(NodeWithValue == nullptr){
    return nullptr;
  }else{
    return (NodeWithValue->value_).get();
  }
}

template <class T>
auto Trie::Put(std::string_view key, T value) const -> Trie {
  // Note that `T` might be a non-copyable type. Always use `std::move` when creating `shared_ptr` on that value.
  //throw NotImplementedException("Trie::Put is not implemented.");

  // You should walk through the trie and create new nodes if necessary. If the node corresponding to the key already
  // exists, you should create a new `TrieNodeWithValue`.
  std::shared_ptr<const TrieNode> root =root_;
  std::shared_ptr<TrieNode> new_root;
  std::shared_ptr<T> value_ptr = std::make_shared<T>(std::move(value));
  //when the key is empty
  if (key.length() == 0) {
    if (root == nullptr) {
      new_root = std::make_shared<TrieNodeWithValue<T>>(value_ptr);
    } else {
      new_root = std::make_shared<TrieNodeWithValue<T>>(root_->children_, value_ptr);
    }
  } else {
    if (root == nullptr) {
      new_root = std::make_shared<TrieNode>();
    } else {
      new_root = std::shared_ptr<TrieNode>(root_->Clone());
    }
  }

  //here I changed a lot with others answer
  std::shared_ptr<TrieNode> cur = new_root;
  int count=0;
  for(char c : key){
    count++;
    auto iter = cur->children_.find(c);
    if(iter == cur->children_.end()){
      if(count==int(key.size())){
        cur -> children_.insert(std::pair<char,std::shared_ptr<const TrieNode>>(c,std::make_shared<TrieNodeWithValue<T>>(value_ptr)));
        break;
      }
      cur -> children_.insert(std::pair<char,std::shared_ptr<const TrieNode>>(c,std::make_shared<TrieNode>()));
      //cur = NewNode;  //I don't know whether it works
    }else{
      if(count==int(key.size())){
        cur -> children_[c]=std::make_shared<TrieNodeWithValue<T>>(cur->children_[c]->children_, value_ptr);
        break;
      }
      cur->children_[c] = std::shared_ptr<TrieNode>(cur->children_[c]->Clone());
    }
    cur = std::const_pointer_cast<TrieNode>(cur->children_[c]);
  }
  return Trie(new_root);
}

auto Trie::Remove(std::string_view key) const -> Trie {
  //throw NotImplementedException("Trie::Remove is not implemented.");

  // You should walk through the trie and remove nodes if necessary. If the node doesn't contain a value any more,
  // you should convert it to `TrieNode`. If a node doesn't have children any more, you should remove it.
  std::shared_ptr<const TrieNode> root =this->root_;
  return 
}

// Below are explicit instantiation of template functions.
//
// Generally people would write the implementation of template classes and functions in the header file. However, we
// separate the implementation into a .cpp file to make things clearer. In order to make the compiler know the
// implementation of the template functions, we need to explicitly instantiate them here, so that they can be picked up
// by the linker.

template auto Trie::Put(std::string_view key, uint32_t value) const -> Trie;
template auto Trie::Get(std::string_view key) const -> const uint32_t *;

template auto Trie::Put(std::string_view key, uint64_t value) const -> Trie;
template auto Trie::Get(std::string_view key) const -> const uint64_t *;

template auto Trie::Put(std::string_view key, std::string value) const -> Trie;
template auto Trie::Get(std::string_view key) const -> const std::string *;

// If your solution cannot compile for non-copy tests, you can remove the below lines to get partial score.

using Integer = std::unique_ptr<uint32_t>;

template auto Trie::Put(std::string_view key, Integer value) const -> Trie;
template auto Trie::Get(std::string_view key) const -> const Integer *;

template auto Trie::Put(std::string_view key, MoveBlocked value) const -> Trie;
template auto Trie::Get(std::string_view key) const -> const MoveBlocked *;

}  // namespace bustub
