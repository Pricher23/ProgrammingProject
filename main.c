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

struct Book {
    int id;
    char title[MAX_TITLE_LENGTH];
    char author[MAX_AUTHOR_LENGTH];
    int copies;
};

struct Book books[MAX_BOOKS];
int numBooks = 0;

struct User {
    char name[MAX_NAME_LENGTH];
    char surename[MAX_SURENAME_LENGTH];
    int rented_books[MAX_RENTED_BOOKS];
    int rented_copies[MAX_RENTED_BOOKS];
    int numRentedBooks;
};

void toLowercase(char* str) {
    for (int i = 0; str[i] != '\0'; i++) {
        str[i] = tolower(str[i]);
    }
}

struct User currentUser;

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
void donate_book();
int check_user(const char *name, const char *surename);
void save_rented_books();
void load_rented_books();

int main() {
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
        printf("6. Donate books\n");
        printf("7. Exit program\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        getchar();
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
                donate_book();
                break;
            case 7:
                printf("Exiting program.\n");
                return 0;
            default:
                printf("Invalid choice. Please enter a number between 1 and 7.\n");
        }
    }

    return 0;
}

void clear_screen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void load_books() {
    FILE *fp = fopen("books.csv", "r");
    if (fp == NULL) {
        printf("Error opening file.\n");
        exit(1);
    }

    char line[1024];
    fgets(line, sizeof(line), fp);

    while (fgets(line, sizeof(line), fp)) {
        char *token = strtok(line, ",");
        int id = atoi(token);
        token = strtok(NULL, ",");
        char title[MAX_TITLE_LENGTH];
        strcpy(title, token);
        token = strtok(NULL, ",");
        char author[MAX_AUTHOR_LENGTH];
        strcpy(author, token);
        token = strtok(NULL, ",");
        int copies = atoi(token);
        books[numBooks].id = id;
        strcpy(books[numBooks].title, title);
        strcpy(books[numBooks].author, author);
        books[numBooks].copies = copies;
        numBooks++;
    }
    fclose(fp);
}

void display_books() {
    printf("Available Books:\n");
    for (int i = 0; i < numBooks; i++) {
        printf("ID: %d, Title: %s, Author: %s, Copies: %d\n", books[i].id, books[i].title, books[i].author, books[i].copies);
    }
}

void search_books() {
    char query[MAX_TITLE_LENGTH];
    printf("Enter title or author to search: ");
    fgets(query, sizeof(query), stdin);
    query[strcspn(query, "\n")] = '\0';

    printf("Search Results:\n");
    for (int i = 0; i < numBooks; i++) {
        if (strstr(books[i].title, query) != NULL || strstr(books[i].author, query) != NULL) {
            printf("ID: %d, Title: %s, Author: %s, Copies: %d\n", books[i].id, books[i].title, books[i].author, books[i].copies);
        }
    }
}

void rent_book() {
    search_books();

    int choice;
    printf("Enter the number of the book you want to rent (or 0 to cancel): ");
    scanf("%d", &choice);

    if (choice == 0 || choice > numBooks) {
        return;
    }

    int selectedBookIndex = choice - 1;

    if (books[selectedBookIndex].copies > 0) {
        int numCopiesToRent;
        printf("Enter the number of copies you want to rent (available: %d): ", books[selectedBookIndex].copies);
        scanf("%d", &numCopiesToRent);

        if (numCopiesToRent <= 0 || numCopiesToRent > books[selectedBookIndex].copies) {
            printf("Invalid number of copies.\n");
            return;
        }

        books[selectedBookIndex].copies -= numCopiesToRent;

        update_rented_books_file(currentUser.name, books[selectedBookIndex].id, numCopiesToRent);

        printf("%d copies of '%s' have been rented successfully.\n", numCopiesToRent, books[selectedBookIndex].title);
    } else {
        printf("Sorry, there are no more copies of '%s' available for rent.\n", books[selectedBookIndex].title);
    }
}

void return_book() {
    see_rented_books();

    FILE *userFile = fopen("user_data.txt", "r");
    if (userFile == NULL) {
        printf("Error opening user_data.txt file for reading.\n");
        return;
    }

    int choice;
    printf("Enter the number of the book you want to return (or 0 to cancel): ");
    scanf("%d", &choice);

    if (choice == 0 || choice > currentUser.numRentedBooks) {
        fclose(userFile);
        return;
    }

    int bookId, numCopies;
    int lineCounter = 0;
    char line[1024];
    FILE *tempFile = fopen("temp_user_data.txt", "w");
    if (tempFile == NULL) {
        printf("Error creating temporary file.\n");
        fclose(userFile);
        return;
    }

    fprintf(tempFile, "Name BookId Copies\n");

    int numCopiesToReturn = 0;

    fseek(userFile, 0, SEEK_SET);
    while (fgets(line, sizeof(line), userFile)) {
        char name[MAX_NAME_LENGTH];
        if (sscanf(line, "%s %d %d", name, &bookId, &numCopies) == 3 && strcmp(name, currentUser.name) == 0) {
            if (++lineCounter == choice) {
                printf("Enter the number of copies you want to return (maximum %d): ", numCopies);
                scanf("%d", &numCopiesToReturn);

                if (numCopiesToReturn < 1 || numCopiesToReturn > numCopies) {
                    printf("Invalid number of copies to return.\n");
                    fclose(userFile);
                    fclose(tempFile);
                    remove("temp_user_data.txt");
                    return;
                }

                numCopies -= numCopiesToReturn;

                if (numCopies > 0) {
                    fprintf(tempFile, "%s %d %d\n", currentUser.name, bookId, numCopies);
                }
                printf("%d copies of Book ID %d have been returned successfully.\n", numCopiesToReturn, bookId);
            } else {
                fprintf(tempFile, "%s %d %d\n", currentUser.name, bookId, numCopies);
            }
        } else {
            fprintf(tempFile, "%s %d %d\n", name, bookId, numCopies);
        }
    }

    fclose(userFile);
    fclose(tempFile);

    remove("user_data.txt");
    rename("temp_user_data.txt", "user_data.txt");

    FILE *booksFile = fopen("books.csv", "r");
    if (booksFile == NULL) {
        printf("Error opening books.csv file for reading.\n");
        return;
    }

    char booksLine[1024];
    char tempBooksLine[1024];
    FILE *tempBooksFile = fopen("temp_books.csv", "w");
    if (tempBooksFile == NULL) {
        printf("Error creating temporary file.\n");
        fclose(booksFile);
        return;
    }

    fgets(booksLine, sizeof(booksLine), booksFile);
    fprintf(tempBooksFile, "%s", booksLine);

    while (fgets(booksLine, sizeof(booksLine), booksFile)) {
        strcpy(tempBooksLine, booksLine);
        int tempId, tempCopies;
        char tempTitle[MAX_TITLE_LENGTH];
        char tempAuthor[MAX_AUTHOR_LENGTH];
        if (sscanf(tempBooksLine, "%d,%[^,],%[^,],%d", &tempId, tempTitle, tempAuthor, &tempCopies) == 4) {
            if (tempId == bookId) {
                tempCopies += numCopiesToReturn;
            }
            fprintf(tempBooksFile, "%d,%s,%s,%d\n", tempId, tempTitle, tempAuthor, tempCopies);
        }
    }

    fclose(booksFile);
    fclose(tempBooksFile);

    remove("books.csv");
    rename("temp_books.csv", "books.csv");
}

void update_rented_books_file(const char *name, int bookId, int numCopies) {
    FILE *userFile = fopen("user_data.txt", "r");
    if (userFile == NULL) {
        printf("Error opening user_data.txt file for reading.\n");
        return;
    }

    struct RentedBook {
        char name[MAX_NAME_LENGTH];
        int bookId;
        int copies;
    } rentedBooks[100];

    int numRentedBooks = 0;

    char line[1024];
    fgets(line, sizeof(line), userFile);
    while (fgets(line, sizeof(line), userFile)) {
        char userName[MAX_NAME_LENGTH];
        int bId, nCopies;
        if (sscanf(line, "%s %d %d", userName, &bId, &nCopies) == 3) {
            strcpy(rentedBooks[numRentedBooks].name, userName);
            rentedBooks[numRentedBooks].bookId = bId;
            rentedBooks[numRentedBooks].copies = nCopies;
            numRentedBooks++;
        }
    }

    fclose(userFile);

    int found = 0;
    for (int i = 0; i < numRentedBooks; i++) {
        if (strcmp(rentedBooks[i].name, name) == 0 && rentedBooks[i].bookId == bookId) {
            rentedBooks[i].copies += numCopies;
            found = 1;
            break;
        }
    }

    if (!found) {
        strcpy(rentedBooks[numRentedBooks].name, name);
        rentedBooks[numRentedBooks].bookId = bookId;
        rentedBooks[numRentedBooks].copies = numCopies;
        numRentedBooks++;
    }

    for (int i = 0; i < numRentedBooks - 1; i++) {
        for (int j = 0; j < numRentedBooks - i - 1; j++) {
            if (strcmp(rentedBooks[j].name, rentedBooks[j + 1].name) > 0) {
                struct RentedBook temp = rentedBooks[j];
                rentedBooks[j] = rentedBooks[j + 1];
                rentedBooks[j + 1] = temp;
            }
        }
    }

    FILE *tempFile = fopen("temp_user_data.txt", "w");
    if (tempFile == NULL) {
        printf("Error creating temporary file.\n");
        return;
    }

    fprintf(tempFile, "Name BookId Copies\n");

    for (int i = 0; i < numRentedBooks; i++) {
        fprintf(tempFile, "%s %d %d\n", rentedBooks[i].name, rentedBooks[i].bookId, rentedBooks[i].copies);
    }

    fclose(tempFile);

    remove("user_data.txt");
    rename("temp_user_data.txt", "user_data.txt");
}

void see_rented_books() {
    FILE *userFile = fopen("user_data.txt", "r");
    if (userFile == NULL) {
        printf("Error opening user_data.txt file for reading.\n");
        return;
    }

    printf("Books rented by %s:\n", currentUser.name);

    int index = 1;
    char line[1024];
    while (fgets(line, sizeof(line), userFile)) {
        char name[MAX_NAME_LENGTH];
        int bookId, numCopies;
        if (sscanf(line, "%s %d %d", name, &bookId, &numCopies) == 3 && strcmp(name, currentUser.name) == 0) {
            printf("%d. Title: %s, Author: %s, Copies Rented: %d\n", index, books[bookId-1].title, books[bookId-1].author, numCopies);
            index++;
        }
    }

    fclose(userFile);

    if (index == 1) {
        printf("You haven't rented any books.\n");
    }
}

int check_user(const char *name, const char *surename) {
    FILE *fp = fopen("users.txt", "r");
    if (fp == NULL) {
        return 0;
    }

    char line[1024];
    while (fgets(line, sizeof(line), fp)) {
        char n[MAX_NAME_LENGTH];
        if (sscanf(line, "%s", n) == 1 && strcmp(name, n) == 0) {
            fclose(fp);
            return 1;
        }
    }

    fclose(fp);
    return 0;
}

void load_user_data() {
    FILE *fp = fopen("user_data.txt", "r");
    if (fp == NULL) {
        return;
    }

    char line[1024];
    while (fgets(line, sizeof(line), fp)) {
        char name[MAX_NAME_LENGTH];
        int bookId, numCopies;
        if (sscanf(line, "%s %d %d", name, &bookId, &numCopies) == 3 && strcmp(name, currentUser.name) == 0) {
            currentUser.rented_books[currentUser.numRentedBooks] = bookId;
            currentUser.rented_copies[currentUser.numRentedBooks] = numCopies;
            currentUser.numRentedBooks++;
        }
    }

    fclose(fp);
}

void save_user_data() {
    FILE *fp = fopen("user_data.txt", "a+");
    if (fp == NULL) {
        printf("Error opening file.\n");
        exit(1);
    }

    for (int i = 0; i < currentUser.numRentedBooks; i++) {
        fprintf(fp, "%s %d %d\n", currentUser.name, currentUser.rented_books[i], currentUser.rented_copies[i]);
    }

    fclose(fp);
}

void login() {
    char name[MAX_NAME_LENGTH];
    char command[100];

    while (1) {
        printf("To login, type: 'login [Name]'\n");
        printf("Enter command: ");
        fgets(command, sizeof(command), stdin);
        command[strcspn(command, "\n")] = '\0';

        if (strncmp(command, "login ", 6) == 0) {
            sscanf(command, "login %s", name);
            break;
        } else {
            printf("Invalid command. Please enter 'login [Name]' to login.\n");
        }
    }

    if (check_user(name, "")) {
        printf("Welcome back, %s!\n", name);
        strcpy(currentUser.name, name);
        load_user_data();
    } else {
        printf("User not found. Registering new user...\n");
        FILE *fp = fopen("users.txt", "a+");
        if (fp == NULL) {
            printf("Error opening file.\n");
            exit(1);
        }
        fprintf(fp, "%s\n", name);
        fclose(fp);
        printf("User registered successfully.\n");
        strcpy(currentUser.name, name);
        currentUser.numRentedBooks = 0;
        save_user_data();
    }
}

void donate_book() {
    char title[MAX_TITLE_LENGTH];
    char author[MAX_AUTHOR_LENGTH];
    int numCopies;

    printf("Enter the title of the book you want to donate: ");
    fgets(title, sizeof(title), stdin);
    title[strcspn(title, "\n")] = '\0';

    printf("Enter the author of the book: ");
    fgets(author, sizeof(author), stdin);
    author[strcspn(author, "\n")] = '\0';

    printf("Enter the number of copies you want to donate: ");
    scanf("%d", &numCopies);

    search_books();

    int found = 0;
    for (int i = 0; i < numBooks; i++) {
        if (strcmp(books[i].title, title) == 0 && strcmp(books[i].author, author) == 0) {
            books[i].copies += numCopies;
            printf("%d copies of '%s' added to the library successfully.\n", numCopies, title);
            found = 1;
            break;
        }
    }

    if (!found) {
        books[numBooks].id = numBooks + 1;
        strcpy(books[numBooks].title, title);
        strcpy(books[numBooks].author, author);
        books[numBooks].copies = numCopies;
        numBooks++;
        printf("%d copies of '%s' added to the library successfully.\n", numCopies, title);
    }

    FILE *booksFile = fopen("books.csv", "w");
    if (booksFile == NULL) {
        printf("Error opening books.csv file for writing.\n");
        exit(1);
    }

    fprintf(booksFile, "ID,Title,Author,Copies\n");

    for (int i = 0; i < numBooks; i++) {
        fprintf(booksFile, "%d,%s,%s,%d\n", books[i].id, books[i].title, books[i].author, books[i].copies);
    }

    fclose(booksFile);
}

