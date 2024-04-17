#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_BOOKS 100
#define MAX_TITLE_LENGTH 100
#define MAX_AUTHOR_LENGTH 100
#define MAX_NAME_LENGTH 50
#define MAX_SURENAME_LENGTH 50
#define MAX_RENTED_BOOKS 10

// Define book structure
struct Book {
    int id;
    char title[MAX_TITLE_LENGTH];
    char author[MAX_AUTHOR_LENGTH];
    int copies;
};

struct Book books[MAX_BOOKS];
int numBooks = 0;

// Define user structure
struct User {
    char name[MAX_NAME_LENGTH];
    char surename[MAX_SURENAME_LENGTH];
    int rented_books[MAX_RENTED_BOOKS];
    int numRentedBooks;
};

void toLowercase(char* str) {
    for (int i = 0; str[i] != '\0'; i++) {
        str[i] = tolower(str[i]);
    }
}

struct User currentUser;

// Function prototypes
void load_books();
void display_books();
void search_books();
void rent_book();
void return_book();
void see_rented_books();
void login();
void save_user_data();
void load_user_data();
void clear_screen();

int main() {
    // Load books from CSV file
    load_books();

    login();

    int choice;
    while (1) {

        printf("\nMain Menu:\n");
        printf("1. Display available books\n");
        printf("2. Search books\n");
        printf("3. Rent a book\n");
        printf("4. Return a book\n");
        printf("5. See rented books\n");
        printf("6. Exit program\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        getchar(); // Consume newline
        clear_screen();
        switch (choice) {
            case 1:
                display_books();
                break;
            case 2:
                search_books();
                break;
            case 3:
                rent_book();
                break;
            case 4:
                return_book();
                break;
            case 5:
                see_rented_books();
                break;
            case 6:
                printf("Exiting program.\n");
                // Save user data before exiting
                save_user_data();
                return 0;
            default:
                printf("Invalid choice. Please enter a number between 1 and 6.\n");
        }
    }

    return 0;
}

// Function to clear the console screen
void clear_screen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

// Function to load books from the CSV file
void load_books() {
    FILE *fp = fopen("books.csv", "r");
    if (fp == NULL) {
        printf("Error opening file.\n");
        exit(1);
    }

    char line[1024];
    // Discard the header line
    fgets(line, sizeof(line), fp);

    // Read each line of the file
    while (fgets(line, sizeof(line), fp)) {
        // Tokenize the line based on commas
        char *token = strtok(line, ",");

        // Extract ID
        int id = atoi(token);

        // Tokenize again to get the title (may contain commas)
        token = strtok(NULL, ",");
        char title[MAX_TITLE_LENGTH];
        strcpy(title, token);

        // Tokenize again to get the author (may contain commas)
        token = strtok(NULL, ",");
        char author[MAX_AUTHOR_LENGTH];
        strcpy(author, token);

        // Extract copies
        token = strtok(NULL, ",");
        int copies = atoi(token);

        // Copy the parsed values into the books array
        books[numBooks].id = id;
        strcpy(books[numBooks].title, title);
        strcpy(books[numBooks].author, author);
        books[numBooks].copies = copies;

        // Increment the number of books
        numBooks++;
    }
    fclose(fp);
}

// Function to display available books
void display_books() {
    printf("Available Books:\n");
    for (int i = 0; i < numBooks; i++) {
        printf("ID: %d, Title: %s, Author: %s, Copies: %d\n", books[i].id, books[i].title, books[i].author, books[i].copies);
    }
}

// Function to search for books by title or author
void search_books() {
    char query[MAX_TITLE_LENGTH];
    printf("Enter title or author to search: ");
    fgets(query, sizeof(query), stdin);
    query[strcspn(query, "\n")] = '\0'; // Remove newline character

    printf("Search Results:\n");
    for (int i = 0; i < numBooks; i++) {
        if (strstr(books[i].title, query) != NULL || strstr(books[i].author, query) != NULL) {
            printf("ID: %d, Title: %s, Author: %s, Copies: %d\n", books[i].id, books[i].title, books[i].author, books[i].copies);
        }
    }
}

void rent_book() {
    char query[MAX_TITLE_LENGTH];
    printf("Enter title or author of the book you want to rent: ");
    fgets(query, sizeof(query), stdin);
    query[strcspn(query, "\n")] = '\0'; // Remove newline character

    int foundBooks[MAX_BOOKS]; // Array to store indexes of found books
    int numFoundBooks = 0;

    // Search for the book by title or author
    for (int i = 0; i < numBooks; i++) {
        if (strstr(books[i].title, query) != NULL || strstr(books[i].author, query) != NULL) {
            foundBooks[numFoundBooks++] = i;
        }
    }

    if (numFoundBooks == 0) {
        printf("No books found matching '%s'.\n", query);
        return;
    }

    printf("Found Books:\n");
    for (int i = 0; i < numFoundBooks; i++) {
        printf("%d. ID: %d, Title: %s, Author: %s, Copies: %d\n", i + 1, books[foundBooks[i]].id, books[foundBooks[i]].title, books[foundBooks[i]].author, books[foundBooks[i]].copies);
    }

    int choice;
    printf("Enter the number of the book you want to rent (or 0 to cancel): ");
    scanf("%d", &choice);

    if (choice == 0 || choice > numFoundBooks) {
        return; // Cancel rent operation or invalid choice
    }

    int selectedBookIndex = foundBooks[choice - 1];

    if (books[selectedBookIndex].copies > 0) {
        int numCopiesToRent;
        printf("Enter the number of copies you want to rent (available: %d): ", books[selectedBookIndex].copies);
        scanf("%d", &numCopiesToRent);
        if (numCopiesToRent <= 0 || numCopiesToRent > books[selectedBookIndex].copies) {
            printf("Invalid number of copies.\n");
            return;
        }
        books[selectedBookIndex].copies -= numCopiesToRent;
        printf("%d copies of '%s' have been rented successfully.\n", numCopiesToRent, books[selectedBookIndex].title);

        // Store rented book data in the user_data file
        FILE *userFile = fopen("user_data.txt", "a"); // Open file in append mode
        if (userFile == NULL) {
            printf("Error opening user_data.txt file for writing.\n");
            exit(1);
        }
        fprintf(userFile, "%s %d %d\n", currentUser.name, selectedBookIndex, numCopiesToRent);
        fclose(userFile);
    } else {
        printf("Sorry, there are no more copies of '%s' available for rent.\n", books[selectedBookIndex].title);
    }
}

void return_book() {
    FILE *userFile = fopen("user_data.txt", "r");
    if (userFile == NULL) {
        printf("Error opening user_data.txt file for reading.\n");
        return;
    }

    printf("Books rented by %s:\n", currentUser.name);

    int found = 0;
    char line[1024];
    int index = 1; // Index for numbering rented books
    while (fgets(line, sizeof(line), userFile)) {
        char name[MAX_NAME_LENGTH];
        int bookId, numCopies;
        if (sscanf(line, "%s %d %d", name, &bookId, &numCopies) == 3 && strcmp(name, currentUser.name) == 0) {
            found = 1;
            printf("%d. ID: %d, Title: %s, Author: %s, Copies Rented: %d\n", index, books[bookId].id, books[bookId].title, books[bookId].author, numCopies);
            index++;
        }
    }

    fclose(userFile);

    if (!found) {
        printf("No books rented by you.\n");
        return;
    }

    int choice;
    printf("Enter the number of the book you want to return (or 0 to cancel): ");
    scanf("%d", &choice);

    if (choice == 0 || choice > index - 1) {
        return;
    }

    userFile = fopen("user_data.txt", "r");
    FILE *tempFile = fopen("temp.txt", "w");
    if (userFile == NULL || tempFile == NULL) {
        printf("Error opening user_data.txt or temp.txt file.\n");
        return;
    }

    found = 0;
    index = 1; // Reset index
    while (fgets(line, sizeof(line), userFile)) {
        char name[MAX_NAME_LENGTH];
        int bookId, numCopies;
        if (sscanf(line, "%s %d %d", name, &bookId, &numCopies) == 3 && strcmp(name, currentUser.name) == 0) {
            if (index == choice) {
                found = 1;
                books[bookId].copies += numCopies; // Increment copies for the returned book
                printf("Book with ID %d has been returned successfully.\n", books[bookId].id);
            } else {
                fprintf(tempFile, "%s %d %d\n", name, bookId, numCopies);
            }
            index++;
        }
    }

    fclose(userFile);
    fclose(tempFile);

    if (!found) {
        printf("Invalid choice.\n");
        remove("temp.txt");
        return;
    }

    remove("user_data.txt");
    rename("temp.txt", "user_data.txt");
}

void see_rented_books() {
    FILE *userFile = fopen("user_data.txt", "r");
    if (userFile == NULL) {
        printf("Error opening user_data.txt file for reading.\n");
        return;
    }

    printf("Books rented by %s:\n", currentUser.name);

    int index = 1; // Index for numbering rented books
    char line[1024];
    while (fgets(line, sizeof(line), userFile)) {
        char name[MAX_NAME_LENGTH];
        int bookId, numCopies;
        if (sscanf(line, "%s %d %d", name, &bookId, &numCopies) == 3 && strcmp(name, currentUser.name) == 0) {
            printf("%d. Title: %s, Author: %s, Copies Rented: %d\n", index, books[bookId].title, books[bookId].author, numCopies);
            index++;
        }
    }

    fclose(userFile);

    if (index == 1) {
        printf("You haven't rented any books.\n");
    }
}



void load_rented_books() {
    FILE *userFile = fopen("user_data.txt", "r");
    if (userFile == NULL) {
        printf("Error opening file.\n");
        exit(1);
    }

    int rentedBookId, numRentedCopies;
    while (fscanf(userFile, "%d %d", &rentedBookId, &numRentedCopies) == 2) {
        for (int i = 0; i < numBooks; i++) {
            if (books[i].id == rentedBookId) {
                printf("ID: %d, Title: %s, Author: %s, Copies Rented: %d\n", rentedBookId, books[i].title, books[i].author, numRentedCopies);
                break;
            }
        }
    }

    fclose(userFile);
}

void login() {
    char name[MAX_NAME_LENGTH], surename[MAX_SURENAME_LENGTH];
    char command[100];

    while (1) {
        printf("To login, type: 'login [Name] [Surename]'\n");
        printf("Enter command: ");
        fgets(command, sizeof(command), stdin);
        command[strcspn(command, "\n")] = '\0'; // Remove newline character

        // Check if the user entered the login command
        if (strncmp(command, "login ", 6) == 0) {
            // Extract name and surename from the command
            sscanf(command, "login %s %s", name, surename);
            break; // Break the loop to proceed with the login process
        } else {
            printf("Invalid command. Please enter 'login [Name] [Surename]' to login.\n");
        }
    }

    FILE *fp = fopen("users.txt", "a+");
    if (fp == NULL) {
        printf("Error opening file.\n");
        exit(1);
    }

    char line[1024];
    while (fgets(line, sizeof(line), fp)) {
        char n[MAX_NAME_LENGTH], s[MAX_SURENAME_LENGTH];
        if (sscanf(line, "%s %s", n, s) == 2 && strcmp(name, n) == 0 && strcmp(surename, s) == 0) {
            strcpy(currentUser.name, name);
            strcpy(currentUser.surename, surename);
            currentUser.numRentedBooks = 0;
            printf("Login successful.\n");

            // Load rented books for the current user
            load_rented_books();

            fclose(fp);
            return;
        }
    }

    fclose(fp);

    char choice;
    printf("User not found. Would you like to register? (Y/N): ");
    scanf(" %c", &choice);
    if (tolower(choice) == 'y') {
        FILE *fp = fopen("users.txt", "a");
        if (fp == NULL) {
            printf("Error opening file.\n");
            exit(1);
        }
        fprintf(fp, "%s %s\n", name, surename);
        fclose(fp);
        printf("User registered successfully.\n");
    } else {
        printf("Exiting program.\n");
        exit(0);
    }
}

// Function to save user data to a file
void save_user_data() {
    FILE *fp = fopen("user_data.txt", "w");
    if (fp == NULL) {
        printf("Error opening file.\n");
        exit(1);
    }

    fprintf(fp, "%s %s\n", currentUser.name, currentUser.surename);
    fprintf(fp, "%d\n", currentUser.numRentedBooks);
    for (int i = 0; i < currentUser.numRentedBooks; i++) {
        fprintf(fp, "%d ", currentUser.rented_books[i]);
    }
    fprintf(fp, "\n");

    fclose(fp);
}

// Function to load user data from a file
void load_user_data() {
    FILE *fp = fopen("user_data.txt", "r");
    if (fp == NULL) {
        // No saved data found
        return;
    }

    fscanf(fp, "%s %s", currentUser.name, currentUser.surename);
    fscanf(fp, "%d", &currentUser.numRentedBooks);
    for (int i = 0; i < currentUser.numRentedBooks; i++) {
        fscanf(fp, "%d", &currentUser.rented_books[i]);
    }

    fclose(fp);
}
