import { Card } from "./card";
import { Player } from "./player";
import { Game } from "./game";
import { Bot } from "./bot";

export class AI {
    bot : Bot | null;
    game: Game;
    me: Player;

    constructor(game: Game, me: Player) {
        this.game = game;
        this.me = me;
    }

    numDiscardsToTake(): number {
        return 0;
    }

    meld(): void {
        // At this point, your hand is updated with drawn cards.
        // How is a run or set actually played?
        // Maybe some bot function...
    }

    discardCard(): Card {
        const [ discard ] = this.game.bot().hand.knownCards;
        return discard;
    }
}