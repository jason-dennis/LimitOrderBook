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
    };

    //max price 200,000 , tick=0.001 => max value =200,000,000
    const int DIM_level1 = 3'125'000; // 200,000,000/64 = 3,125,000
    const int DIM_level2 = 48'828;  // 3,125,000/64 = ~48,828
    const int DIM_level3 = 762; // 48,828/64 = ~762
    const int DIM_level4 = 11; // 762/64 = ~11

    std::vector<uint64_t>level1_;
    std::vector<uint64_t>level2_;
    std::vector<uint64_t>level3_;
    std::vector<uint64_t>level4_;

    std::unordered_map<uint64_t,LinkedList>PriceList_;
    std::unordered_map<int,Node*>NodeLocation;

public:

    BinaryOrderBook():level1_(DIM_level1), level2_(DIM_level2),
                      level3_(DIM_level3), level4_(DIM_level4){}
    ~BinaryOrderBook() override = default;

    int CalcInd(int x);
    int CalcBit(int x);

    void AddOrder(const Order& order) override;
    void CancelOrder(int order_id) override;
    void UpdateQuantity(int order_id,int new_quantity) override;

    const Order* GetBestBid() const override;
    const Order* GetBestAsk() const override;

    bool IsBidEmpty() const override;
    bool IsAskEmpty() const override;
    bool CanFillQuantityAsks(int Quantity, int Price) const override;
    bool CanFillQuantityBids(int Quantity, int Price) const override;

    void PopBestBid() override;
    void PopBestAsk() override;
};

#endif //LIMITORDERBOOK_BINARYORDERBOOK_H