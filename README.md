# PuzzleGame

This is a Windows-only implementation of the classic move the box/Sokoban puzzle game.  You can load levels using custom `.slc` files (a variation on XML). 

## Features

* **Custom Level Loading:** Supports loading levels from `.slc` files.
* **Configurable Display:** Adjust screen width, height, and toggle fullscreen mode via command-line arguments.
* **Controls:** Use arrow keys for movement, `Esc` to exit, `N` for the next level, and `R` to retry the current level.

## Command-Line Arguments

* `--input <filename>`: Specify the `.slc` file containing levels. **(Required)**
* `--width <pixels>`: Set the screen width (default: 1920).
* `--height <pixels>`: Set the screen height (default: 1080).
* `--fullscreen`: Enable fullscreen mode (default: false).

## External Libraries

* **msxml & comutil:** For loading `.slc` files.
* **SDL:** Core game rendering.
* **SDL_Font:** Text rendering.
* **SDL_Image:** Asset loading.

## Credits

* **Level Pack:** [https://www.sourcecode.se/sokoban/levels.php](https://www.sourcecode.se/sokoban/levels.php) (Download more levels here!)
* **Game Assets:** [https://kenney.nl/assets/sokoban](https://kenney.nl/assets/sokoban)
* **Font:** Roboto from [https://www.fontsquirrel.com/fonts/roboto](https://www.fontsquirrel.com/fonts/roboto)

## How to Play

1. Make sure you have the required libraries installed.
2. Run the game from the command line, providing the path to your `.slc` level file using the `--input` argument.
3. Use the arrow keys to move the player and push boxes.
4. The goal is to push all boxes onto the target locations.
5. Press `N` to move to the next level or `R` to retry the current one.
6. Press `Esc` to exit the game.