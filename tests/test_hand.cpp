// tests/test_hand.cpp: tests for Hand totals, soft/hard, bust, blackjack
#include "check.h"
#include "hand.h"

void test_hand_total() {
    {   // A, 10: blackjack -> soft 21
        Hand h;
        h.addCard(Card{Suit::Hearts, Rank::Ace});
        h.addCard(Card{Suit::Diamonds, Rank::Ten});
        CHECK(h.total() == 21);
        CHECK(h.isSoft());
        CHECK(h.isBlackjack());
    }
    {   // 5, 6: no ace -> hard 11
        Hand h;
        h.addCard(Card{Suit::Clubs, Rank::Five});
        h.addCard(Card{Suit::Spades, Rank::Six});
        CHECK(h.total() == 11);
        CHECK(!h.isSoft());
        CHECK(!h.isBlackjack());
    }
    {   // A, 6: basic promotion -> soft 17
        Hand h;
        h.addCard(Card{Suit::Hearts, Rank::Ace});
        h.addCard(Card{Suit::Clubs, Rank::Six});
        CHECK(h.total() == 17);
        CHECK(h.isSoft());
    }
    {   // A, 6, 10: promotion would bust -> hard 17
        Hand h;
        h.addCard(Card{Suit::Hearts, Rank::Ace});
        h.addCard(Card{Suit::Clubs, Rank::Six});
        h.addCard(Card{Suit::Spades, Rank::Ten});
        CHECK(h.total() == 17);
        CHECK(!h.isSoft());
        CHECK(!h.isBust());
    }
    {   // A, A: only one ace is promoted -> soft 12
        Hand h;
        h.addCard(Card{Suit::Hearts, Rank::Ace});
        h.addCard(Card{Suit::Spades, Rank::Ace});
        CHECK(h.total() == 12);
        CHECK(h.isSoft());
    }
    {   // A, A, 9: promotion lands exactly on 21 -> soft 21, not blackjack
        Hand h;
        h.addCard(Card{Suit::Hearts, Rank::Ace});
        h.addCard(Card{Suit::Spades, Rank::Ace});
        h.addCard(Card{Suit::Clubs, Rank::Nine});
        CHECK(h.total() == 21);
        CHECK(h.isSoft());
        CHECK(!h.isBlackjack());
    }
    {   // K, Q, A: ace must stay 1 -> hard 21, three cards so not blackjack
        Hand h;
        h.addCard(Card{Suit::Hearts, Rank::King});
        h.addCard(Card{Suit::Spades, Rank::Queen});
        h.addCard(Card{Suit::Clubs, Rank::Ace});
        CHECK(h.total() == 21);
        CHECK(!h.isSoft());
        CHECK(!h.isBlackjack());
        CHECK(!h.isBust());
    }
    {   // K, Q, 5: bust
        Hand h;
        h.addCard(Card{Suit::Hearts, Rank::King});
        h.addCard(Card{Suit::Spades, Rank::Queen});
        h.addCard(Card{Suit::Clubs, Rank::Five});
        CHECK(h.total() == 25);
        CHECK(h.isBust());
    }
    {   // empty hand: nothing dealt yet
        Hand h;
        CHECK(h.total() == 0);
        CHECK(!h.isSoft());
        CHECK(!h.isBlackjack());
    }
}
