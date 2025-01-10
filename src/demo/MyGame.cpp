#include "MyGame.h"


MyGame::MyGame() : AbstractGame(), score(0), lives(3), numKeys(5), gameWon(false) {
	TTF_Font * font = ResourceManager::loadFont("res/fonts/arial.ttf", 72);
	gfx->useFont(font);
	gfx->setFullscreen(true);
	gfx->setVerticalSync(true);
	backgroundTexture = gfx->loadTexture("../res/tex/easterBG.jpg");
	backgroundNormal = gfx->loadTexture("../res/tex/background_norm.jpg");
	

    for (int i = 0; i < numKeys; i++) {
        std::shared_ptr<GameKey> k = std::make_shared<GameKey>();
        k->isAlive = true;
        k->pos = Point2(getRandom(0, 750), getRandom(0, 550));
        gameKeys.push_back(k);
    }
}



MyGame::~MyGame() {

}

void MyGame::handleKeyEvents() {
	int speed = 10;

	if (eventSystem->isPressed(Key::W)) {
		velocity.y = +speed;
	}

	if (eventSystem->isPressed(Key::S)) {
		velocity.y = -speed;
	}

	if (eventSystem->isPressed(Key::A)) {
		velocity.x = -speed;
	}

	if (eventSystem->isPressed(Key::D)) {
		velocity.x = speed;
	}

	if (eventSystem->isPressed(Key::SPACE)) {
		gfx->randomizeLightColor(0);
	}
}

void MyGame::update() {
	// Get the current mouse position
	int mouseX, mouseY;
	SDL_GetMouseState(&mouseX, &mouseY);  // Get mouse position in screen coordinates

	mouseY =  DEFAULT_WINDOW_HEIGHT-mouseY;

	// Update the box's position to follow the cursor
	lightPosX = mouseX;
	lightPosY = mouseY;
}
void MyGame::render() {
	gfx->clearScreen();

	GLuint shaderProgram = gfx->getShaderProgram();
	glBindTexture(GL_TEXTURE_2D, backgroundTexture);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, backgroundNormal);
	glUniform1i(glGetUniformLocation(shaderProgram, "normalMap"), 1);

	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f); glVertex2f(-1.0f, -1.0f); // Bottom-left
	glTexCoord2f(1.0f, 0.0f); glVertex2f(1.0f, -1.0f);  // Bottom-right
	glTexCoord2f(1.0f, 1.0f); glVertex2f(1.0f, 1.0f);   // Top-right
	glTexCoord2f(0.0f, 1.0f); glVertex2f(-1.0f, 1.0f);  // Top-left
	glEnd();

	Vector2i lightPos = { lightPosX, lightPosY };
	gfx->drawSpotlight(lightPos);
	gfx->showScreen();
}