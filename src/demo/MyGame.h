#ifndef __TEST_GAME_H__
#define __TEST_GAME_H__

#include "../engine/AbstractGame.h"

struct GameKey {
	Point2 pos;
	bool isAlive;
};

class MyGame : public AbstractGame {
	private:
		int lightPosX;
		int lightPosY;

		Vector2i velocity;

		std::vector<std::shared_ptr<GameKey>> gameKeys;

		/* GAMEPLAY */
		int score, numKeys, lives;
		bool gameWon;
		GLuint backgroundTexture;
		GLuint backgroundNormal;

		void handleKeyEvents();
		void update();
		void render();
	public:
        MyGame();
		~MyGame();
};

#endif