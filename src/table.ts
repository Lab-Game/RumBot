import { Meld } from './meld';
import { Hand } from './hand';

export class Table {
    melds: Meld[] = [];

    constructor() {}

    findMelds(hand: Hand) {
        let melds: Meld[] = [];

        for (const card of hand.cards) {
            let possibleMelds = Meld.cardToMelds.get(card)!;
            for (const meld of possibleMelds) {
                if (meld.cards.every(c => hand.hasCard(c))) {
                    melds.push(meld);
                }
            }
        }

        for (const meld of this.melds) {
            for (const ext of meld.extensions) {
                if (ext.cards.every(c => hand.hasCard(c))) {
                    melds.push(ext);
                }
            }
        }

        return melds;
    }

    oneMeld(hand: Hand): Meld | undefined {
        for (const card of hand.cards) {
            let possibleMelds = Meld.cardToMelds.get(card)!;
            for (const meld of possibleMelds) {
                if (meld.cards.every(c => hand.hasCard(c))) {
                    return meld;
                }
            }
        }

        for (const meld of this.melds) {
            for (const ext of meld.extensions) {
                if (ext.cards.every(c => hand.hasCard(c))) {
                    return ext;
                }
            }
        }

        return undefined;
    }
}
