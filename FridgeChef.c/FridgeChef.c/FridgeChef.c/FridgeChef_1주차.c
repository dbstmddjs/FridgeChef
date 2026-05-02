#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#endif

#define MAX_ITEMS    100
#define MAX_RECIPES  10
#define MAX_ING      5
#define IMMINENT     3
#define FILENAME     "inventory.txt"

/* 재료 구조체 */
typedef struct {
    char name[50];
    char category[20];
    char location[10];
    int  quantity;
    char expiry[11];
} Item;

/* 레시피 구조체 */
typedef struct {
    char name[50];
    char ingredients[MAX_ING][50];
    int  ingCount;
} Recipe;

Item   inventory[MAX_ITEMS];
int    itemCount = 0;
Recipe recipes[MAX_RECIPES];
int    recipeCount = 0;

void listItems(void);
void addItem(void);
void deleteItem(void);
void checkAlert(void);
void showRecipes(void);

void printLine(void) {
    printf("================================================================\n");
}

int countImminent(void) {
    return 0;
}

void showMenu(void) {
    int imm = countImminent();
    printLine();
    printf("        FridgeChef - 냉장고 재고 & 레시피 관리\n");
    printLine();
    printf("\n");
    printf("  데이터 불러오는 중...\n");
    printf("  재료 %d 로드 완료료.\n", itemCount);
    printf("\n");
    printf("  +------------------------------------------------------+\n");
    printf("  |                    메인 메뉴                         |\n");
    printf("  +------------------------------------------------------+\n");
    printf("  |  1.  재료 목록 보기                                  |\n");
    printf("  |  2.  재료 추가                                       |\n");
    printf("  |  3.  재료 삭제                                       |\n");
    if (imm > 0)
        printf("  |  4.  유통기한 임박 확인     !! %d개 임박!             |\n", imm);
    else
        printf("  |  4.  유통기한 임박 확인                              |\n");
    printf("  |  5.  레시피 추천                                     |\n");
    printf("  |  0.  종료                                            |\n");
    printf("  +------------------------------------------------------+\n");
    printf("\n  선택 >> ");
}

void listItems(void) {
    printf("\n  [재료 목록] \n");
    printf("  Enter 를 누르면 메인 메뉴로 돌아갑니다...");
    getchar(); getchar();
}

void addItem(void) {
    printf("\n  [재료 추가] \n");
    printf("  Enter 를 누르면 메인 메뉴로 돌아갑니다...");
    getchar(); getchar();
}

void deleteItem(void) {
    printf("\n  [재료 삭제] \n");
    printf("  Enter 를 누르면 메인 메뉴로 돌아갑니다...");
    getchar(); getchar();
}

void checkAlert(void) {
    printf("\n  [유통기한 임박 확인] \n");
    printf("  Enter 를 누르면 메인 메뉴로 돌아갑니다...");
    getchar(); getchar();
}

void showRecipes(void) {
    printf("\n  [레시피 추천]\n");
    printf("  Enter 를 누르면 메인 메뉴로 돌아갑니다...");
    getchar(); getchar();
}

int main(void) {
    int  choice;
    char confirm;

#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    while (1) {
#ifdef _WIN32
        system("cls");
#else
        system("clear");
#endif
        showMenu();
        scanf("%d", &choice);

        switch (choice) {
            case 1: listItems();   break;
            case 2: addItem();     break;
            case 3: deleteItem();  break;
            case 4: checkAlert();  break;
            case 5: showRecipes(); break;
            case 0:
                printf("\n  정말 종료하시겠습니까? (y/n) >> ");
                scanf(" %c", &confirm);
                if (confirm == 'y' || confirm == 'Y') {
                    printLine();
                    printf("    FridgeChef 를 종료합니다. 감사합니다!\n");
                    printLine();
                    return 0;
                }
                break;
            default:
                printf("\n  잘못된 입력입니다. 다시 선택하세요\n");
                printf("  Enter 를 누르면 메인 메뉴로 돌아갑니다...");
                getchar(); getchar();
                break;
        }
    }
    return 0;
}