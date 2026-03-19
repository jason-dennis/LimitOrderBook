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
        const std::shared_ptr<Order> order_;
        Node* next_;
        Node* prev_;
    public:
        Node(const std::shared_ptr<Order> &order): order_(order), next_(nullptr),prev_(nullptr){}

        Node* GetNext() const {return next_;}
        Node* GetPrev() const {return prev_;}

        std::shared_ptr<Order> GetOrder() const {return order_;}

        void SetPrev(Node* prev){prev_ = prev;}
        void SetNext(Node* next) {next_ = next;}

    };
    class LinkedList {
    private:
        Node* head_;
        Node* tail_;
    public:

        LinkedList(): head_(nullptr), tail_(nullptr){}

        Node* AddNode(std::shared_ptr<Order> &order) {
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
    class ListMap {
        static constexpr uint64_t PAGE_SIZE = 64;
        static constexpr uint64_t NUM_PAGES = 20'000'000 / PAGE_SIZE + 1;
 
        std::vector<LinkedList*> pages_;
 
    public:
        ListMap() : pages_(NUM_PAGES, nullptr) {}
 
        ~ListMap() {
            for (auto* p : pages_)
                delete[] p;
        }
 

        ListMap(const ListMap&) = delete;
        ListMap& operator=(const ListMap&) = delete;

        ListMap(ListMap&& other) noexcept : pages_(std::move(other.pages_)) {}
        ListMap& operator=(ListMap&& other) noexcept {
            if (this!= &other) {
                for (auto* p : pages_) delete[] p;
                pages_ = std::move(other.pages_);
            }
            return *this;
        }
 
        LinkedList& operator[](uint64_t price) {
            uint64_t page_idx = price / PAGE_SIZE;
            uint64_t slot_idx = price % PAGE_SIZE;
            if (!pages_[page_idx]) {
                pages_[page_idx] = new LinkedList[PAGE_SIZE]();
            }
            return pages_[page_idx][slot_idx];
        }
    };
//
    const int DIM_level1 = 312'501;
    const int DIM_level2 =  4'884;
    const int DIM_level3 = 77;
    const int DIM_level4 = 2;

//    const int DIM_level1 = 3'125'000; // 200,000,000/64 = 3,125,000
//    const int DIM_level2 = 48'828;  // 3,125,000/64 = ~48,828
//    const int DIM_level3 = 762; // 48,828/64 = ~762
//    const int DIM_level4 = 11; // 762/64 = ~11

    std::vector<uint64_t>level1_bid;
    std::vector<uint64_t>level2_bid;
    std::vector<uint64_t>level3_bid;
    std::vector<uint64_t>level4_bid;

    std::vector<uint64_t>level1_ask;
    std::vector<uint64_t>level2_ask;
    std::vector<uint64_t>level3_ask;
    std::vector<uint64_t>level4_ask;

    ListMap BidList_;
    ListMap AskList_;
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

    void AddOrder(std::shared_ptr<Order> order) override;
    void CancelOrder(int order_id) override;
    void UpdateQuantity(int order_id,int new_quantity) override;
    void DeleteBid(int order_id,Node* node, uint64_t Price);
    void DeleteAsk(int order_id, Node* node, uint64_t Price);

    const std::shared_ptr<Order> GetBestBid()  override;
    const std::shared_ptr<Order> GetBestAsk()  override;

    bool IsBidEmpty() const override;
    bool IsAskEmpty() const override;
    bool CanFillQuantityAsks(int Quantity, uint64_t Price) const override;
    bool CanFillQuantityBids(int Quantity, uint64_t Price) const override;

    void PopBestBid() override;
    void PopBestAsk() override;
};

#endif //LIMITORDERBOOK_BINARYORDERBOOK_H