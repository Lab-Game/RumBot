import { Card } from "./card";

export class Meld {

    static cardToMelds: Map<Card, Meld[]> = new Map();

    static init() {
        // Set melds are simpler than run melds.
        // For each card in the deck, there is a three-card set consisting of that
        // card and cards of adjacent suits with equal value, e.g. 3C 3D 3H.
        // This triplet can be extended only by a singleton with of the remaining
        // suit and same value, e.g. 3S.
        // No further melds can be added to this singleton.
        // The points for a meld are the sum of the points of its cards, where aces
        // are worth 15 points.
        for (const card of Card.deck) {
            let singleSet = new Meld([ card.nextOfValue().nextOfValue() ]);
            let tripleSet = new Meld([ card.prevOfValue(), card, card.nextOfValue() ]);
            tripleSet.adds.push(singleSet);
            this.cardToMelds.set(card, [ tripleSet ]);
        }

        // Run melds are more complex, largely because of aces can be high or low.
        // Initially ignore the high / low difference.
        let singleRuns = Card.deck.map(card => new Meld([card]));
        for (const meld of singleRuns) {
            let card = meld.cards[0];
            if (card.isAce()) {
                meld.lower = new Meld([card]);
                meld.lower.points -= 10;
            }
        }

        for (const meld of singleRuns) {
            const card = meld[0];
            if (!card.isAce()) {
                if (card.value() >= Card.FOUR_VALUE.value()) {
                    meld.adds.push(singleRuns[card.nextInSuit().index]);
                }
                if (card.value() == TWO_VALUE.value()) {
                    meld.adds.push(singleRuns[card.prevInSuit().index].lower!);
                } else if (card.value() <= Card.fromString("JH").value()) {
                    meld.adds.push(singleRuns[card.prevInSuit().index]);
                }
            }
        }

        for (const card of Card.deck) {
            if (!card.isAce()) {
                let tripleRun = new Meld([card.prevInSuit(), card, card.nextInSuit()]);
                if (!tripleRun.cards[0].isAce()) {
                    tripleRun.adds.push(singleRuns[card.prevInSuit().index]);
                }
                if (!tripleRun.cards[2].isAce()) {
                    tripleRun.adds.push(singleRuns[card.nextInSuit().index]);
                }
                this.cardToMelds.set(card, [ tripleRun ]);
            }
        }
    }

    cards: Card[];
    points: number;
    adds: Meld[] = [];
    lower: Meld | undefined;

    constructor(cards: Card[]) {
        this.cards = cards;
        this.points = cards.reduce((sum, card) => sum + card.points(), 0);
        this.adds = [];
        this.lower = undefined;
    }

    toString(): string {
        return `[${Card.cardsToString(this.cards)}]`;
    }
}