    #include <SFML/Graphics.hpp>
    #include <iostream>
    #include <vector>
    #include <string>
    #include <cstdlib>
    #include <sstream>

    #define MAX_DISKS 10
    #define ROD_WIDTH 10
    #define DISK_HEIGHT 20

    const int COMPLETION_WINDOW_WIDTH = 800;  // Set width
    const int COMPLETION_WINDOW_HEIGHT = 600;

    bool isFullscreen = false; 
    bool gameStarted = false; 
    bool buttonClicked = false;

    void showCompletionWindow();
    void hanoiOptimalMoves(int n, char fromRod, char toRod, char auxRod, std::vector<std::string>& optimalPath);

    // Disk structure
    struct Disk {
        int width;
        sf::RectangleShape shape;
    };

    // Rod structure
    struct Rod {
        std::vector<Disk> disks;
        int top = 0;

        void push(const Disk &disk) {
            disks.push_back(disk);
            top++;
        }

        Disk pop() {
            Disk disk = disks.back();
            disks.pop_back();
            top--;
            return disk;
        }

        bool isEmpty() const {
            return top == 0;
        }

        Disk peek() const {
            return disks.back();
        }
    };

    // Global variables
    Rod A, B, C;
    int numDisks = 0;
    int movesCount = 0;
    sf::Font font;
    sf::Text readyText; // Global text for "Are you ready to play!?"

    // Store player moves and invalid moves
    std::vector<std::string> playerMoves;
    std::vector<std::string> invalidMoves;

    // Input box variables
    std::string userInput;
    bool inputActive = true;

    // Button class for user interaction
    class Button {
    public:
        sf::RectangleShape shape;
        sf::Text text;
        sf::Color originalColor; // Store the original color
        sf::Color clickedColor; // Color when clicked

        Button(float x, float y, const std::string &label) {
            shape.setSize(sf::Vector2f(150, 40));
            shape.setFillColor(sf::Color::Cyan);
            originalColor = sf::Color::Cyan; // Store the original color
            clickedColor = sf::Color::Red; // Define clicked color
            shape.setPosition(x, y);

            text.setFont(font);
            text.setString(label);
            text.setCharacterSize(18);
            text.setFillColor(sf::Color::Black);
            text.setPosition(x + 10, y + 5);
        }

        bool isClicked(const sf::Vector2i &mousePos) const {
            return shape.getGlobalBounds().contains(static_cast<sf::Vector2f>(mousePos));
        }

        void draw(sf::RenderWindow &window) const {
            window.draw(shape);
            window.draw(text);
        }

        void setClickedColor() {
            shape.setFillColor(clickedColor);
        }

        void resetColor() {
            shape.setFillColor(originalColor);
        }
    };


    std::string formatCompletionSummary() {
        std::stringstream output;
        output << "Congratulations! You solved the Tower of Hanoi puzzle with " << numDisks << " disks.\n";
        output << "Total moves taken by you: " << movesCount << "\n";
        output << "Least number of moves required: " << (1 << numDisks) - 1 << "\n\n";
        
        output << "Your Moves:\n";
        for (const auto& move : playerMoves) {
            output << move << "\n";
        }

        output << "\nInvalid Moves:\n";
        for (const auto& move : invalidMoves) {
            output << move << "\n";
        }

        output << "\nOptimal Path:\n";
        std::vector<std::string> optimalPath;
        hanoiOptimalMoves(numDisks, 'A', 'C', 'B', optimalPath);
        for (const auto& move : optimalPath) {
            output << move << "\n";
        }

        return output.str();
    }

    // Function to create the "Are you ready to play!?" text
    sf::Text createReadyText(float buttonX) {
        sf::Text readyText;
        readyText.setFont(font);
        readyText.setString("Are you ready to play!?");
        readyText.setCharacterSize(16);
        readyText.setFillColor(sf::Color::White);
        readyText.setPosition(buttonX + 160, 350); // Position it beside the start button

        return readyText;
    }

    // Function to attempt moving a disk from one rod to another
    void attemptMove(Rod* fromRod, Rod* toRod, const std::string& fromLabel, const std::string& toLabel) {
        if (fromRod->isEmpty()) {
            std::cerr << "Invalid move: Source rod is empty!" << std::endl;
            invalidMoves.push_back("Invalid move: Tried to move disk from rod " + fromLabel + " to rod " + toLabel);
            return;
        }
        if (!toRod->isEmpty() && fromRod->peek().width > toRod->peek().width) {
            std::cerr << "Invalid move: Cannot place a larger disk on a smaller one!" << std::endl;
            invalidMoves.push_back("Invalid move: Tried to move disk from rod " + fromLabel + " to rod " + toLabel);
            return;
        }

        Disk disk = fromRod->pop();
        toRod->push(disk);
        movesCount++;

        playerMoves.push_back("Move disk " + std::to_string(disk.width / 20) + " from rod " + fromLabel + " to rod " + toLabel);
    }
    void checkGameCompletion() {
        if (C.top == numDisks) {
            std::cout << "Congratulations! You solved the Tower of Hanoi puzzle with " << numDisks << " disks.\n";
            std::cout << "Total moves taken by you: " << movesCount << "\n";
            std::cout << "Least number of moves required: " << (1 << numDisks) - 1 << "\n";
            showCompletionWindow();
        }
    }

    void showCompletionWindow() {
        sf::RenderWindow completionWindow(sf::VideoMode(COMPLETION_WINDOW_WIDTH, COMPLETION_WINDOW_HEIGHT), "Game Completion Summary");

        sf::Text completionText;
        completionText.setFont(font);
        completionText.setCharacterSize(14); // Medium character size
        completionText.setFillColor(sf::Color::White);
        completionText.setString(formatCompletionSummary());
        completionText.setPosition(10, 10);

        // Adjust line spacing to fit text better
        completionText.setLineSpacing(1.5f);

        // Track vertical scroll offset
        float scrollOffset = 0.0f;
        const float scrollSpeed = 30.0f; // Adjust scrolling speed

        // Measure the total height of the text
        sf::FloatRect textBounds = completionText.getGlobalBounds();
        float totalTextHeight = textBounds.height;

        while (completionWindow.isOpen()) {
            sf::Event event;
            while (completionWindow.pollEvent(event)) {
                if (event.type == sf::Event::Closed) {
                    completionWindow.close();
                }

                // Handle mouse wheel scrolling
                if (event.type == sf::Event::MouseWheelScrolled) {
                    if (event.mouseWheelScroll.delta > 0) {
                        // Scroll up
                        scrollOffset += scrollSpeed;
                    } else {
                        // Scroll down
                        scrollOffset -= scrollSpeed;
                    }

                    // Limit scrolling to not exceed text bounds
                    if (scrollOffset > 0) {
                        scrollOffset = 0; // Don't scroll past the top
                    }
                    if (scrollOffset < -totalTextHeight + COMPLETION_WINDOW_HEIGHT - 20) {
                        scrollOffset = -totalTextHeight + COMPLETION_WINDOW_HEIGHT - 20; // Don't scroll past the bottom
                    }

                    completionText.setPosition(10, scrollOffset + 10); // Adjust text position based on scroll
                }
            }

            completionWindow.clear();
            completionWindow.draw(completionText);
            completionWindow.display();
        }
    }

    void hanoiOptimalMoves(int n, char fromRod, char toRod, char auxRod, std::vector<std::string>& optimalPath) {
        if (n == 1) {
            optimalPath.push_back("Move disk 1 from rod " + std::string(1, fromRod) + " to rod " + std::string(1, toRod));
            return;
        }
        hanoiOptimalMoves(n - 1, fromRod, auxRod, toRod, optimalPath);
        optimalPath.push_back("Move disk " + std::to_string(n) + " from rod " + std::string(1, fromRod) + " to rod " + std::string(1, toRod));
        hanoiOptimalMoves(n - 1, auxRod, toRod, fromRod, optimalPath);
    }

    // Buttons for moving disks between rods
    float buttonX = 800 - 160; // x position for buttons in the top right corner
    Button moveAToBButton(buttonX, 50, "Move A to B");
    Button moveAToCButton(buttonX, 100, "Move A to C");
    Button moveBToAButton(buttonX, 150, "Move B to A");
    Button moveBToCButton(buttonX, 200, "Move B to C");
    Button moveCToAButton(buttonX, 250, "Move C to A");
    Button moveCToBButton(buttonX, 300, "Move C to B");
    Button startButton(buttonX, 350, "Start Game"); // Button to start the game
    //Button startButton(buttonX, 350, "Start Game");

    // Function to initialize rods
    void initializeRods() {
        for (int i = numDisks; i > 0; i--) {
            Disk disk;
            disk.width = i * 20;  
            disk.shape.setSize(sf::Vector2f(disk.width, DISK_HEIGHT));

            int r = std::rand() % 256;
            int g = std::rand() % 256;
            int b = std::rand() % 256;
            disk.shape.setFillColor(sf::Color(r, g, b));

            disk.shape.setPosition((800 / 6) - (disk.width / 2), 600 - (A.top * DISK_HEIGHT) - 100);
            A.push(disk);
        }
    }

    // Function to draw a rod
    void drawRod(sf::RenderWindow &window, float x) {
        sf::Vertex rod[] = {
            sf::Vertex(sf::Vector2f(x, 600 - 100), sf::Color::White),
            sf::Vertex(sf::Vector2f(x, 600 - 20), sf::Color::White)
        };
        window.draw(rod, 2, sf::Lines);
    }

    // Function to draw disks
    void drawDisk(sf::RenderWindow &window, const Disk &disk) {
        window.draw(disk.shape);
    }

    // Function to draw the current state of the game
    void drawState(sf::RenderWindow &window, sf::Text &rulesText) {
        window.clear(sf::Color::Black);

        drawRod(window, 800 / 6);
        drawRod(window, 800 / 2);
        drawRod(window, 5 * 800 / 6);

        // Draw disks for each rod
        for (int i = 0; i < A.top; i++) {
            Disk &disk = A.disks[i];
            disk.shape.setPosition((800 / 6) - (disk.width / 2), 600 - (i * DISK_HEIGHT) - 100);
            drawDisk(window, disk);
        }
        for (int i = 0; i < B.top; i++) {
            Disk &disk = B.disks[i];
            disk.shape.setPosition((800 / 2) - (disk.width / 2), 600 - (i * DISK_HEIGHT) - 100);
            drawDisk(window, disk);
        }
        for (int i = 0; i < C.top; i++) {
            Disk &disk = C.disks[i];
            disk.shape.setPosition((5 * 800 / 6) - (disk.width / 2), 600 - (i * DISK_HEIGHT) - 100);
            drawDisk(window, disk);
        }

        // Draw buttons in the top right corner
        moveAToBButton.draw(window);
        moveAToCButton.draw(window);
        moveBToAButton.draw(window);
        moveBToCButton.draw(window);
        moveCToAButton.draw(window);
        moveCToBButton.draw(window);

        // Draw rules text at the top left corner
        window.draw(rulesText);

        // Draw "Are you ready to play!?" message beside the start button
        if (!gameStarted) {
            startButton.draw(window); // Draw start button if game not started
            window.draw(readyText); // Draw the readiness text
        }   
    }

    // Function to display the rules of the game
    sf::Text createRulesText() {
        sf::Text rulesText;
        rulesText.setFont(font);
        rulesText.setCharacterSize(16);
        rulesText.setFillColor(sf::Color::White);

        std::string rules = "Tower of Hanoi Rules:\n";
        rules += "1. Move only one disk at a time.\n";
        rules += "2. A disk can only be placed on a larger disk.\n";
        rules += "3. Move all disks from rod A to rod C.\n";
        rules += "\nTotal number of moves required: 2^n - 1\n";
        rulesText.setString(rules);
        rulesText.setPosition(10, 10); // Top left corner position

        return rulesText;
    }

    // Function to handle user input
    void handleUserInput(sf::Event &event) {
        if (inputActive) {
            if (event.type == sf::Event::TextEntered) {
                if (event.text.unicode < 128) { // Check if the input is a printable character
                    if (event.text.unicode == '\b') { // Backspace
                        if (!userInput.empty()) {
                            userInput.pop_back();
                        }
                    } else if (event.text.unicode == '\r') { // Enter key
                        numDisks = std::stoi(userInput);
                        inputActive = false; // Disable input
                    } else {
                        userInput += static_cast<char>(event.text.unicode);
                    }
                }
            }
        }
    }

    // Function to draw the input box
    // Function to draw the input box
    void drawInputBox(sf::RenderWindow &window) {
        sf::RectangleShape inputBox(sf::Vector2f(300, 50));
        inputBox.setFillColor(sf::Color::White);
        inputBox.setPosition(250, 250);

        sf::Text inputText;
        inputText.setFont(font);
        inputText.setString(userInput);
        inputText.setCharacterSize(24);
        inputText.setFillColor(sf::Color::Black);
        inputText.setPosition(255, 255);

        // Create and position the prompt text
        sf::Text promptText;
        promptText.setFont(font);
        promptText.setString("Enter the number of disks: ");
        promptText.setCharacterSize(24);
        promptText.setFillColor(sf::Color::White);
        promptText.setPosition(250, 210); // Position above the input box

        window.draw(inputBox);
        window.draw(promptText); // Draw the prompt text
        window.draw(inputText);
    }

    void toggleFullscreen(sf::RenderWindow &window) {
        isFullscreen = !isFullscreen;

        if (isFullscreen) {
            window.create(sf::VideoMode::getDesktopMode(), "Tower of Hanoi", sf::Style::Fullscreen);
        } else {
            window.create(sf::VideoMode(800, 600), "Tower of Hanoi", sf::Style::Close);
        }
    }

    // Main function
    int main() {
        // Initially, create the window in windowed mode
        sf::RenderWindow window(sf::VideoMode(800, 600), "Tower of Hanoi");

        if (!font.loadFromFile("arial.ttf")) {
            std::cerr << "Failed to load font\n";
            return -1;
        }

        // Create rules text to be drawn at the top left corner
        sf::Text rulesText = createRulesText();

        // Add the readiness text
        sf::Text readyText;
        readyText.setFont(font);
        readyText.setString("Are you ready to play!?");
        readyText.setCharacterSize(16);
        readyText.setFillColor(sf::Color::White);
        readyText.setPosition(buttonX + 160, 350); // Position it beside the start button

        // Main loop
        while (window.isOpen()) {
            sf::Event event;
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed) {
                    window.close();
                }

                // Detect F11 keypress to toggle fullscreen
                if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::F11) {
                    toggleFullscreen(window);
                }

                // Handle user input
                handleUserInput(event);

                // Handle button clicks only if input is not active
                if (!inputActive) {
                    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                        
                        // Start button click
                        if (!gameStarted && startButton.isClicked(mousePos)) {
                            startButton.setClickedColor(); // Change color on click
                            initializeRods(); // Initialize the rods with the number of disks
                            gameStarted = true; // Set game as started
                            buttonClicked = true; // Set button clicked
                        }

                        // Only allow moving disks if the game has started
                        if (gameStarted) {
                            if (moveAToBButton.isClicked(mousePos)) {
                                moveAToBButton.setClickedColor();
                                attemptMove(&A, &B, "A", "B");
                                buttonClicked = true; // Set button clicked
                            } else if (moveAToCButton.isClicked(mousePos)) {
                                moveAToCButton.setClickedColor();
                                attemptMove(&A, &C, "A", "C");
                                buttonClicked = true; // Set button clicked
                            } else if (moveBToAButton.isClicked(mousePos)) {
                                moveBToAButton.setClickedColor();
                                attemptMove(&B, &A, "B", "A");
                                buttonClicked = true; // Set button clicked
                            } else if (moveBToCButton.isClicked(mousePos)) {
                                moveBToCButton.setClickedColor();
                                attemptMove(&B, &C, "B", "C");
                                buttonClicked = true; // Set button clicked
                            } else if (moveCToAButton.isClicked(mousePos)) {
                                moveCToAButton.setClickedColor();
                                attemptMove(&C, &A, "C", "A");
                                buttonClicked = true; // Set button clicked
                            } else if (moveCToBButton.isClicked(mousePos)) {
                                moveCToBButton.setClickedColor();
                                attemptMove(&C, &B, "C", "B");
                                buttonClicked = true; // Set button clicked
                            }
                            checkGameCompletion();
                        }
                    }
                }
            }

            window.clear();

            // Reset button colors after processing
            if (buttonClicked) {
                if (!gameStarted) {
                    startButton.resetColor(); // Reset start button color
                } else {
                    moveAToBButton.resetColor();
                    moveAToCButton.resetColor();
                    moveBToAButton.resetColor();
                    moveBToCButton.resetColor();
                    moveCToAButton.resetColor();
                    moveCToBButton.resetColor();
                }
                buttonClicked = false; // Reset button clicked state
            }

            if (inputActive) {
                drawInputBox(window); // Draw the input box to enter the number of disks
            } else {
                if (!gameStarted) {
                    startButton.draw(window); // Draw start button if game not started
                    window.draw(readyText); // Draw the readiness text
                } else {
                    drawState(window, rulesText); // Draw the game state along with rules
                }
            }

            window.display();
        } // End of while (window.isOpen())

        return 0; // End of main()
    } // End of main function