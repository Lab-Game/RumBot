import { Card, Meld } from './card';
import { Hand } from './hand';

export class Table {
    melds: Meld[] = [];

    constructor() {}

    findMelds(hand: Hand) {
        let melds: Meld[] = [];

        for (const card of hand.cards) {
            if (card.tripleSet.cards.every(c => hand.hasCard(c))) {
                melds.push(card.tripleSet);
            }
            if (card.tripleRun?.cards.every(c => hand.hasCard(c))) {
                melds.push(card.tripleRun);
            }
        }

        for (const meld of this.melds) {
            for (const addition of meld.addOns) {
                if (addition.cards.every(c => hand.hasCard(c))) {
                    melds.push(addition);
                }
            }
        }

        return melds;
    }

    oneMeld(hand: Hand): Meld | undefined {
        for (const card of hand.cards) {
            if (card.tripleSet.cards.every(c => hand.hasCard(c))) {
                return card.tripleSet;
            }
            if (card.tripleRun?.cards.every(c => hand.hasCard(c))) {
                return card.tripleRun;
            }
        }

        for (const meld of this.melds) {
            for (const addition of meld.addOns) {
                if (addition.cards.every(c => hand.hasCard(c))) {
                    return addition;
                }
            }
        }

        return undefined;
    }
}
