#include "GraphicsEngine.h"
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <vector>

SDL_Renderer * GraphicsEngine::renderer = nullptr;

GraphicsEngine::GraphicsEngine() : fpsAverage(0), fpsPrevious(0), fpsStart(0), fpsEnd(0), drawColor(toSDLColor(0, 0, 0, 255)) {

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	window = SDL_CreateWindow("The X-CUBE 2D Game Engine",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);


	// CATCHES IF WINDOW NOT CREATED
	if (nullptr == window)
		throw EngineException("Failed to create window", SDL_GetError());


	SDL_GLContext glContext = SDL_GL_CreateContext(window);
	// CATCHES IF GL CONTEXT NOT CREATED
	// Also logs error but i cant find it in the console..
	if (nullptr == glContext) {
		std::cerr << "Failed to create OpenGL context: " << SDL_GetError() << std::endl;
		throw EngineException("Failed to create OpenGL context", SDL_GetError());
	}

	if (glewInit() != GLEW_OK) {
		std::cerr << "Failed to initialize GLEW" << std::endl;
		throw EngineException("Failed to initialize GLEW", "GLEW error");
	}

	std::string vertexShaderSource = loadShaderSource("../src/engine/spotlight.vert");
	std::string fragmentShaderSource = loadShaderSource("../src/engine/spotlight.frag");

	shaderProgram = createShaderProgram(vertexShaderSource, fragmentShaderSource);

	glBindAttribLocation(shaderProgram, 0, "position");
	glBindAttribLocation(shaderProgram, 1, "texCoord");


	// Define a full-screen quad (normalized coordinates)
	float vertices[] = {
		-1.0f, -1.0f, 0.0f, 0.0f, // Bottom left corner
		 1.0f, -1.0f, 1.0f, 0.0f, // Bottom right corner
		-1.0f,  1.0f, 0.0f, 1.0f, // Top left corner

		-1.0f,  1.0f, 0.0f, 1.0f, // Top left corner
		 1.0f, -1.0f, 1.0f, 0.0f, // Bottom right corner
		 1.0f,  1.0f, 1.0f, 1.0f  // Top right corner
	};

	// Create and bind VAO and VBO
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Define vertex attributes
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0); // Unbind VAO

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	// CATCHES IF RENDERER NOT CREATED
	if (nullptr == renderer)
		throw EngineException("Failed to create renderer", SDL_GetError());

	SDL_GL_MakeCurrent(window, glContext);
	SDL_GL_SetSwapInterval(1);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	setupOrthographicProjection(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT);
	glViewport(0, 0, DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT);
	drawColor = toSDLColor(255/255.0f, 255 / 255.0f, 255 / 255.0f, 255 / 255.0f);

	// although not necessary, SDL doc says to prevent hiccups load it before using
	if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) != IMG_INIT_PNG)
		throw EngineException("Failed to init SDL_image - PNG", IMG_GetError());

	if (TTF_Init() < 0)
		throw EngineException("Failed to init SDL_ttf", TTF_GetError());
}

GraphicsEngine::~GraphicsEngine() {
#ifdef __DEBUG
	debug("GraphicsEngine::~GraphicsEngine() started");
#endif

	IMG_Quit();
	TTF_Quit();
	SDL_DestroyWindow(window);
	SDL_Quit();

#ifdef __DEBUG
	debug("GraphicsEngine::~GraphicsEngine() finished");
#endif
}

void GraphicsEngine::setWindowTitle(const char * title) {
	SDL_SetWindowTitle(window, title);
#ifdef __DEBUG
	debug("Set window title to:", title);
#endif
}

void GraphicsEngine::setWindowTitle(const std::string & title) {
	SDL_SetWindowTitle(window, title.c_str());
#ifdef __DEBUG
	debug("Set window title to:", title.c_str());
#endif
}

void GraphicsEngine::setWindowIcon(const char *iconFileName) {
	SDL_Surface * icon = IMG_Load(iconFileName);
	if (nullptr == icon) {
		std::cout << "Failed to load icon: " << iconFileName << std::endl;
		std::cout << "Aborting: GraphicsEngine::setWindowIcon()" << std::endl;
		return;
	}
	SDL_SetWindowIcon(window, icon);
#ifdef __DEBUG
	debug("Set Window Icon to", iconFileName);
#endif
	SDL_FreeSurface(icon);
}

void GraphicsEngine::setupOrthographicProjection(int width, int height) {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// Parameters: left, right, bottom, top. Y-Axis inverted!!!! (VERY IMPORTANT MAX REMEMBER THIS OK!!)
	glOrtho(0, width, height, 0, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}


void GraphicsEngine::setFullscreen(bool b) {
	SDL_SetWindowFullscreen(window, b ? SDL_WINDOW_FULLSCREEN_DESKTOP : SDL_WINDOW_MAXIMIZED);
}

void GraphicsEngine::setVerticalSync(bool b) {
	if (!SDL_SetHint(SDL_HINT_RENDER_VSYNC, b ? "1" : "0")) {
		std::cout << "Failed to set VSYNC" << std::endl;
		std::cout << SDL_GetError() << std::endl;
	}
#ifdef __DEBUG
	debug("Current VSYNC:", SDL_GetHint(SDL_HINT_RENDER_VSYNC));
#endif
}

void GraphicsEngine::setDrawColor(const SDL_Color& color) {
	glColor4ub(color.r, color.g, color.b, color.a);
	drawColor = color;
}

void GraphicsEngine::setWindowSize(const int &w, const int &h) {
	SDL_SetWindowSize(window, w, h);
	SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
#ifdef __DEBUG
	debug("Set Window W", w);
	debug("Set Window H", h);
#endif
}

Dimension2i GraphicsEngine::getCurrentWindowSize() {
	int w, h;
	SDL_GetWindowSize(window, &w, &h);
	return Dimension2i(w, h);
}

Dimension2i GraphicsEngine::getMaximumWindowSize() {
	SDL_DisplayMode current;
	if (SDL_GetCurrentDisplayMode(0, &current) == 0) {
		return Dimension2i(current.w, current.h);
	}
	else {
		std::cout << "Failed to get window data" << std::endl;
		std::cout << "GraphicsEngine::getMaximumWindowSize() -> return (0, 0)" << std::endl;
		return Dimension2i();
	}
}

void GraphicsEngine::showInfoMessageBox(const std::string & info, const std::string & title) {
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, title.c_str(), info.c_str(), window);
}

void GraphicsEngine::clearScreen() {
	glClearColor(20.0f / 255.0f, 20.0f / 255.0f, 20.0f / 255.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
}

void GraphicsEngine::showScreen() {
	SDL_GL_SwapWindow(window);
}

void GraphicsEngine::useFont(TTF_Font * _font) {
	if (nullptr == _font) {
#ifdef __DEBUG
		debug("GraphicsEngine::useFont()", "font is null");
#endif
		return;
	}

	font = _font;
}

GLuint GraphicsEngine::getShaderProgram() const {
	return shaderProgram;
}

void GraphicsEngine::setFrameStart() {
	fpsStart = SDL_GetTicks();
}

void GraphicsEngine::adjustFPSDelay(const Uint32 &delay) {
	fpsEnd = SDL_GetTicks() - fpsStart;
	if (fpsEnd < delay) {
		SDL_Delay(delay - fpsEnd);
	}

	Uint32 fpsCurrent = 1000 / (SDL_GetTicks() - fpsStart);
	fpsAverage = (fpsCurrent + fpsPrevious + fpsAverage * 8) / 10;	// average, 10 values / 10
	fpsPrevious = fpsCurrent;
}

Uint32 GraphicsEngine::getAverageFPS() {
	return fpsAverage;
}

SDL_Texture * GraphicsEngine::createTextureFromSurface(SDL_Surface * surf) {
	return SDL_CreateTextureFromSurface(renderer, surf);
}

void GraphicsEngine::setDrawScale(const Vector2f & v) {
	SDL_RenderSetScale(renderer, v.x, v.y);
}

// Utility function to load shader source code from a file
std::string GraphicsEngine::loadShaderSource(const std::string& filepath) {
	std::ifstream file(filepath);
	if (!file.is_open()) {
		std::cerr << "Failed to open shader file: " << filepath << std::endl;
		return "";
	}
	std::stringstream buffer;
	buffer << file.rdbuf();
	std::cout << buffer.str();
	return buffer.str();
}

// Utility function to compile a shader and check for errors
GLuint GraphicsEngine::compileShader(GLenum shaderType, const std::string& shaderSource)
{
	GLuint shader = glCreateShader(shaderType);
	if (shader == 0) {
		std::cerr << "Error creating shader of type " << shaderType << std::endl;
		return 0;
	}

	// Check if the shader source is empty before passing it

	if (shaderSource.empty()) {
		std::cerr << "ERROR: Shader source is empty!" << std::endl;
		glDeleteShader(shader);
		return 0;
	}

	std::cout << "Shader Source: \n" << shaderSource << std::endl;
	const char* source = shaderSource.c_str();
	glShaderSource(shader, 1, &source, nullptr);
	glCompileShader(shader);

	// Check for compilation errors
	GLint success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		GLint logLength;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
		std::vector<char> log(logLength);
		glGetShaderInfoLog(shader, logLength, nullptr, log.data());
		std::cerr << "ERROR: Shader compilation failed:\n" << log.data() << std::endl;
		glDeleteShader(shader);
		return 0; // Return 0 to indicate failure
	}

	return shader;
}


GLuint GraphicsEngine::createShaderProgram(const std::string& vertexShaderSource, const std::string& fragmentShaderSource)
{
	// Check if the shader source is empty before creating shaders
	if (vertexShaderSource.empty() || fragmentShaderSource.empty()) {
		std::cerr << "ERROR: Shader source is empty!" << std::endl;
		return 0; // Or handle the error appropriately
	}

	GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
	if (vertexShader == 0) return 0; // Compilation failed, return 0

	GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
	if (fragmentShader == 0) return 0; // Compilation failed, return 0

	GLuint shaderProgram = glCreateProgram();
	if (0 == shaderProgram) {
	std::cerr << "ERROR: Shader program empty";
	}
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	// Check for linking errors
	GLint success;
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		GLint logLength;
		glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &logLength);
		std::vector<char> log(logLength);
		glGetProgramInfoLog(shaderProgram, logLength, nullptr, log.data());
		std::cerr << "Program linking failed: " << log.data() << std::endl;
		glDeleteProgram(shaderProgram); // Clean up the program if linking failed
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
		return 0; // Return 0 to indicate failure
	}

	// Clean up shaders as they are no longer needed after linking
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return shaderProgram;
}

/* ALL DRAW FUNCTIONS */
/* overloads explicitly call SDL funcs for better performance hopefully */

void GraphicsEngine::drawRect(const Rectangle2& rect) {
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f); // Solid white
	GLfloat mainVertices[] = {
		rect.x, rect.y,                      // Bottom-left
		rect.x + rect.w, rect.y,            // Bottom-right
		rect.x + rect.w, rect.y + rect.h,   // Top-right
		rect.x, rect.y + rect.h             // Top-left
	};

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_FLOAT, 0, mainVertices);
	glDrawArrays(GL_LINE_LOOP, 0, 4);
	glDisableClientState(GL_VERTEX_ARRAY);
}


void GraphicsEngine::drawSpotlight(Vector2i& lightPos) {

	// Use the shader program
	glUseProgram(shaderProgram);

	// Set the uniform values
	glUniform2f(glGetUniformLocation(shaderProgram, "lightPos"), lightPos.x, lightPos.y);
	glUniform1f(glGetUniformLocation(shaderProgram, "lightRadius"), 800.0f);
	glUniform1f(glGetUniformLocation(shaderProgram, "lightIntensity"), 2.0f);
	glUniform4f(glGetUniformLocation(shaderProgram, "lightColor"), 0.9f, 0.9f, 1.0f, 1.0f);
	glUniform1f(glGetUniformLocation(shaderProgram, "ditheringAmount"), 1.0f);

	glUniform4f(glGetUniformLocation(shaderProgram, "ambientColor"), 0.0f, 0.0f, 0.0f, 1.0f);

	// Bind the VAO and VBO
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	// Draw the spotlight 
	glDrawArrays(GL_TRIANGLES, 0, 6);  // Adjust according to the number of vertices in your VAO

	// Unbind the VAO and VBO after drawing
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void GraphicsEngine::drawRect(const Rectangle2 & rect, const SDL_Color & color) {
	glColor4ub(color.r, color.g, color.b, color.a);

	glBegin(GL_LINE_LOOP);
	glVertex2f(rect.x, rect.y);
	glVertex2f(rect.x + rect.w, rect.y);
	glVertex2f(rect.x + rect.w, rect.y + rect.h);
	glVertex2f(rect.x, rect.y + rect.h);
	glEnd();

	glColor4ub(drawColor.r, drawColor.g, drawColor.b, drawColor.a);
}

void GraphicsEngine::drawRect(SDL_Rect * rect, const SDL_Color & color) {
	glColor4ub(color.r, color.g, color.b, color.a);

	glBegin(GL_LINE_LOOP);
	glVertex2f(rect->x, rect->y);
	glVertex2f(rect->x + rect->w, rect->y);
	glVertex2f(rect->x + rect->w, rect->y + rect->h);
	glVertex2f(rect->x, rect->y + rect->h);
	glEnd();

	glColor4ub(drawColor.r, drawColor.g, drawColor.b, drawColor.a);
}

void GraphicsEngine::drawRect(SDL_Rect * rect) {
	glColor4ub(drawColor.r, drawColor.g, drawColor.b, drawColor.a);

	glBegin(GL_LINE_LOOP);
	glVertex2f(rect->x, rect->y);
	glVertex2f(rect->x + rect->w, rect->y);
	glVertex2f(rect->x + rect->w, rect->y + rect->h);
	glVertex2f(rect->x, rect->y + rect->h);
	glEnd();
}

void GraphicsEngine::drawRect(const int &x, const int &y, const int &w, const int &h) {
	glColor4ub(drawColor.r, drawColor.g, drawColor.b, drawColor.a);

	glBegin(GL_LINE_LOOP);
	glVertex2f(x, y);
	glVertex2f(x + w, y);
	glVertex2f(x + w, y + h);
	glVertex2f(x, y + h);
	glEnd();
}

void GraphicsEngine::fillRect(SDL_Rect * rect) {
	//glColor4ub(drawColor.r, drawColor.g, drawColor.b, drawColor.a);

	// Begin drawing a filled rectangle using GL_QUADS
	glBegin(GL_QUADS);
	glVertex2f(rect->x, rect->y);               // Bottom left
	glVertex2f(rect->x + rect->w, rect->y);     // Bottom right
	glVertex2f(rect->x + rect->w, rect->y + rect->h); // Top right
	glVertex2f(rect->x, rect->y + rect->h);     // Top left
	glEnd();
}

void GraphicsEngine::fillRect(const int &x, const int &y, const int &w, const int &h) {
	glColor4ub(drawColor.r, drawColor.g, drawColor.b, drawColor.a); // Set the color

	glBegin(GL_QUADS); // Start drawing a quadrilateral
	glVertex2f(x, y);          // Bottom left
	glVertex2f(x + w, y);      // Bottom right
	glVertex2f(x + w, y + h);  // Top right
	glVertex2f(x, y + h);      // Top left
	glEnd();
}

void GraphicsEngine::drawCircle(const Point2 & center, const float & radius) {
	glColor4ub(drawColor.r, drawColor.g, drawColor.b, drawColor.a);

	// Begin drawing an ellipse using points or a line loop
	glBegin(GL_LINE_LOOP); // or GL_POINTS for individual points
	for (float angle = 0.0f; angle < 2 * M_PI; angle += 0.01f) { // Increase precision by lowering step
		float x = center.x + radius * cos(angle);
		float y = center.y + radius * sin(angle);
		glVertex2f(x, y);
	}
	glEnd();
}

void GraphicsEngine::drawEllipse(const Point2 & center, const float & radiusX, const float & radiusY, const SDL_Color & color) {
	glColor4ub(color.r, color.g, color.b, color.a);

	// Begin drawing an ellipse using points or a line loop
	glBegin(GL_LINE_LOOP); // or GL_POINTS for individual points
	for (float angle = 0.0f; angle < 2 * M_PI; angle += 0.01f) { // Increase precision by lowering step
		float x = center.x + radiusX * cos(angle);
		float y = center.y + radiusY * sin(angle);
		glVertex2f(x, y);
	}
	glEnd();
}

GLuint GraphicsEngine::loadTexture(const std::string& filepath) {
	SDL_Surface* surface = IMG_Load(filepath.c_str());
	if (!surface) {
		std::cerr << "Failed to load texture: " << filepath << " - " << IMG_GetError() << std::endl;
		return 0;
	}

	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	// Set texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Upload texture data
	GLint format = (surface->format->BytesPerPixel == 4) ? GL_RGBA : GL_RGB;
	glTexImage2D(GL_TEXTURE_2D, 0, format, surface->w, surface->h, 0, format, GL_UNSIGNED_BYTE, surface->pixels);

	glBindTexture(GL_TEXTURE_2D, 0);
	SDL_FreeSurface(surface);

	return textureID;
}