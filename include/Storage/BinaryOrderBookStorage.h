//
// Created by denni on 3/14/2026.
//

#ifndef LIMITORDERBOOK_BINARYORDERBOOK_H
#define LIMITORDERBOOK_BINARYORDERBOOK_H
#include "../Storage/IOrderBookStorage.h"
#include<unordered_map>
/**
 * @class BinaryOrderBook
 * @brief High-performance order book implementation using a binary-indexed (bitmap) structure.
 * Uses a 4-level bitmap hierarchy for O(1) best bid/ask lookups,
 * with linked lists at each price level for FIFO order management.
 * Optimized for low-latency trading with paged memory allocation.
 */
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
    /// Bitmap dimensions per level (price range / 64 per level)
    /// Max PRICE 20.000.000 -> 200.000 *tick(100)
    const int DIM_level1 = 312'501;
    const int DIM_level2 =  4'884;
    const int DIM_level3 = 77;
    const int DIM_level4 = 2;

    std::vector<uint64_t>level1_bid;///< Level 1 bitmap for bids
    std::vector<uint64_t>level2_bid;///< Level 2 bitmap for bids
    std::vector<uint64_t>level3_bid;///< Level 3 bitmap for bids
    std::vector<uint64_t>level4_bid;///< Level 4 bitmap for bids

    std::vector<uint64_t>level1_ask;///< Level 1 bitmap for asks
    std::vector<uint64_t>level2_ask;///< Level 2 bitmap for asks
    std::vector<uint64_t>level3_ask;///< Level 3 bitmap for asks
    std::vector<uint64_t>level4_ask;///< Level 4 bitmap for asks

    ListMap BidList_;///< Price-to-order-queue map for bids
    ListMap AskList_;///< Price-to-order-queue map for asks
    std::unordered_map<int,Node*>BidLocation;///< Order ID to node lookup for bids
    std::unordered_map<int,Node*>AskLocation;///< Order ID to node lookup for asks

public:

    BinaryOrderBook()
    : level1_bid(DIM_level1), level2_bid(DIM_level2),
      level3_bid(DIM_level3), level4_bid(DIM_level4),
      level1_ask(DIM_level1), level2_ask(DIM_level2),
      level3_ask(DIM_level3), level4_ask(DIM_level4)
    {}
    ~BinaryOrderBook() override = default;

    /// @brief Calculates the index for a bitmap level.
    uint64_t CalcInd(uint64_t x);

    /// @brief Calculates the bit position within a bitmap word.
    uint64_t CalcBit(uint64_t x);

    /**
     * @brief Adds a new order to the book.
     * @param order The order object to be registered.
     */
    void AddOrder(std::shared_ptr<Order> order) override;

    /**
     * @brief Removes an order from the book based on its ID.
     * @param order_id The unique identifier of the order to be removed.
     */
    void CancelOrder(int order_id) override;

    /**
     * @brief Modifies the volume of an existing order.
     * @param order_id The unique identifier of the order.
     * @param new_quantity The updated quantity.
     */
    void UpdateQuantity(int order_id,int new_quantity) override;

    /// @brief Removes a bid order from the book.
    void DeleteBid(int order_id,Node* node, uint64_t Price);

    /// @brief Removes an ask order from the book.
    void DeleteAsk(int order_id, Node* node, uint64_t Price);

    /**
     * @brief Accesses the highest-priced Buy order.
     * @return Pointer to the best Bid, or nullptr if the side is empty.
     */
    const std::shared_ptr<Order> GetBestBid()  override;

    /**
     * @brief Accesses the lowest-priced Ask order.
     * @return Pointer to the best Ask, or nullptr if the side is empty.
     */
    const std::shared_ptr<Order> GetBestAsk()  override;

    /// @brief Returns the top x bid price levels.
    std::vector<std::shared_ptr<Order>> GetBestBids(int x)  override;

    /// @brief Returns the top x ask price levels.
    std::vector<std::shared_ptr<Order>> GetBestAsks(int x)  override;

    /**
     * @brief Checks for the presence of Buy orders.
     * @return true if the Bid side is empty.
     */
    bool IsBidEmpty() const override;

     /**
     * @brief Checks for the presence of Sell orders.
     * @return true if the Ask side is empty.
     */
    bool IsAskEmpty() const override;


    bool CanFillQuantityAsks(int Quantity, uint64_t Price)  override;


    bool CanFillQuantityBids(int Quantity, uint64_t Price)  override;

    /**
         * @brief Removes the top-priority Buy order from the book.
         */
    void PopBestBid() override;

    /**
         * @brief Removes the top-priority Buy order from the book.
         */
    void PopBestAsk() override;
};

#endif //LIMITORDERBOOK_BINARYORDERBOOK_H