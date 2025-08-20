import { Card, Meld } from './card';
import { Hand } from './hand';

export class Table {
    melds: Set<Meld> = new Set<Meld>();

    constructor() {}

    playMeld(meld) {
        this.melds.add(meld)
    }

    unplayMeld(meld) {
        this.melds.delete(meld);
    }

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

    allMelds(hand: Hand) {
        let plays: Meld[] = [];
        let noPlays = new Set<Meld>();
        let points = 0;

        let maxPoints = this.allMeldsRec(hand, plays, noPlays, points);
        console.log(`Max points from all melds: ${maxPoints}`);

        return maxPoints;
    }

    allMeldsRec(hand: Hand, plays: Meld[], noPlays: Set<Meld>, points: number): number {
        let indent = "    ".repeat(plays.length + noPlays.size);
        let melds = this.findMelds(hand);
        for (const meld of melds) {
            if (noPlays.has(meld)) {
                // We already decided not to play this one.
                continue;
            }

            // We have a playable meld.  Try playing and also try not playing.
            console.log(indent, "Playing meld:", meld.toString());
            hand.removeCards(meld.cards);
            this.playMeld(meld);
            plays.push(meld);
            let playPoints = this.allMeldsRec(hand, plays, noPlays, points + meld.points);
            plays.pop();
            this.unplayMeld(meld);
            hand.addCards(meld.cards);

            console.log(indent, "Not playing meld:", meld.toString());
            noPlays.add(meld);
            let noPlayPoints = this.allMeldsRec(hand, plays, noPlays, points);
            noPlays.delete(meld);

            return Math.max(playPoints, noPlayPoints);
        }

        // No more plays possible.
        return points;
    }
}
