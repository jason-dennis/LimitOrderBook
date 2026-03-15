//
// Created by denni on 3/14/2026.
//

#ifndef LIMITORDERBOOK_BINARYORDERBOOK_H
#define LIMITORDERBOOK_BINARYORDERBOOK_H
#include "../Storage/IOrderBookStorage.h"
#include<unordered_map>

class BinaryOrderBook: public IOrderBook {
private:
    class Node {
    private:
        const Order* order_;
        Node* next_;
        Node* prev_;
    public:
        Node(const Order* order): order_(order), next_(nullptr),prev_(nullptr){}

        Node* GetNext() const {return next_;}
        Node* GetPrev() const {return prev_;}

        const Order* GetOrder() const {return order_;}

        void SetPrev(Node* prev){prev_ = prev;}
        void SetNext(Node* next) {next_ = next;}
        void SetOrder(const Order* order){order_ = order;}

    };
    class LinkedList {
    private:
        Node* head_;
        Node* tail_;
    public:

        LinkedList(): head_(nullptr), tail_(nullptr){}

        Node* AddNode(const Order* order) {
            Node* node= new Node(order);
            if (!tail_) {
                head_=tail_=node;
            }
            else {
                tail_->SetNext(node);
                node->SetPrev(tail_);
                tail_ = node;
            }
            return node;
        }

        void DeleteNode(Node* node) {
            if (node->GetPrev()) {
                node->GetPrev()->SetNext(node->GetNext());
            }
            if (node->GetNext()) {
                node->GetNext()->SetPrev(node->GetPrev());
            }

            if (node == head_) {
                head_=node->GetNext();
            }
            if (node == tail_) {
                tail_=node->GetPrev();
            }
            delete node;
        }

        bool IsEmpty() const {
            return head_ == nullptr;
        }

        Node* GetHead() const{
            return head_;
        }
    };

    const int DIM_level1 = 312'505;
    const int DIM_level2 = 48'885;
    const int DIM_level3 = 80;
    const int DIM_level4 = 3;

    std::vector<uint64_t>level1_bid;
    std::vector<uint64_t>level2_bid;
    std::vector<uint64_t>level3_bid;
    std::vector<uint64_t>level4_bid;

    std::vector<uint64_t>level1_ask;
    std::vector<uint64_t>level2_ask;
    std::vector<uint64_t>level3_ask;
    std::vector<uint64_t>level4_ask;

    std::unordered_map<uint64_t,LinkedList>BidList_;
    std::unordered_map<uint64_t,LinkedList>AskList_;
    std::unordered_map<int,Node*>BidLocation;
    std::unordered_map<int,Node*>AskLocation;

public:

    BinaryOrderBook()
    : level1_bid(DIM_level1), level2_bid(DIM_level2),
      level3_bid(DIM_level3), level4_bid(DIM_level4),
      level1_ask(DIM_level1), level2_ask(DIM_level2),
      level3_ask(DIM_level3), level4_ask(DIM_level4)
    {}
    ~BinaryOrderBook() override = default;

    uint64_t CalcInd(uint64_t x);
    uint64_t CalcBit(uint64_t x);

    void AddOrder(const Order& order) override;
    void CancelOrder(int order_id) override;
    void UpdateQuantity(int order_id,int new_quantity) override;
    void DeleteBid(int order_id,Node* node, uint64_t Price);
    void DeleteAsk(int order_id, Node* node, uint64_t Price);

    const Order* GetBestBid() const override;
    const Order* GetBestAsk() const override;

    bool IsBidEmpty() const override;
    bool IsAskEmpty() const override;
    bool CanFillQuantityAsks(int Quantity, uint64_t Price) const override;
    bool CanFillQuantityBids(int Quantity, uint64_t Price) const override;

    void PopBestBid() override;
    void PopBestAsk() override;
};

#endif //LIMITORDERBOOK_BINARYORDERBOOK_H