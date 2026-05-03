#define _CRT_SECURE_NO_WARNINGS  /* Visual Studio에서 구형 함수 경고 비활성화 */

#include <stdio.h>    /* 표준 입출력 (printf, scanf 등) */
#include <stdlib.h>   /* 표준 라이브러리 (system 등) */
#include <string.h>   /* 문자열 처리 함수 (strcmp, strcpy 등) */
#include <time.h>     /* 시간 관련 함수 (유통기한 비교에 활용 예정) */

#ifdef _WIN32
#include <windows.h>  /* Windows 전용 API (콘솔 코드페이지 설정 등) */
#endif

/* ── 상수 정의 ───────────────────────────────────────────────── */
#define MAX_ITEMS    100   /* 저장 가능한 최대 재료 수 */
#define MAX_RECIPES  10    /* 저장 가능한 최대 레시피 수 */
#define MAX_ING      5     /* 레시피 1개당 최대 재료 수 */
#define IMMINENT     3     /* 유통기한 임박 기준 (남은 일수) */
#define FILENAME     "inventory.txt"  /* 재고 데이터를 저장/불러올 파일명 */

/* ── 재료 구조체 ─────────────────────────────────────────────── */
typedef struct {
    char name[50];       /* 재료 이름 (예: "당근") */
    char category[20];   /* 분류 (예: "채소", "육류") */
    char location[10];   /* 보관 위치 (예: "냉장", "냉동") */
    int  quantity;       /* 수량 */
    char expiry[11];     /* 유통기한 문자열 (형식: "YYYY-MM-DD") */
} Item;

/* ── 레시피 구조체 ───────────────────────────────────────────── */
typedef struct {
    char name[50];               /* 레시피 이름 (예: "된장찌개") */
    char ingredients[MAX_ING][50]; /* 필요 재료 목록 (최대 MAX_ING개) */
    int  ingCount;               /* 실제 필요 재료 수 */
} Recipe;

/* ── 전역 변수 ───────────────────────────────────────────────── */
Item   inventory[MAX_ITEMS];  /* 재료 목록 배열 */
int    itemCount = 0;         /* 현재 등록된 재료 수 */
Recipe recipes[MAX_RECIPES];  /* 레시피 목록 배열 */
int    recipeCount = 0;       /* 현재 등록된 레시피 수 */

/* ── 함수 전방 선언 ──────────────────────────────────────────── */
void listItems(void);    /* 재료 목록 출력 */
void addItem(void);      /* 재료 추가 */
void deleteItem(void);   /* 재료 삭제 */
void checkAlert(void);   /* 유통기한 임박 재료 확인 */
void showRecipes(void);  /* 레시피 추천 */

/* 구분선 출력 함수 */
void printLine(void) {
    printf("================================================================\n");
}

/*
 * countImminent - 유통기한이 IMMINENT일 이내로 남은 재료 수를 반환한다.
 * 현재는 미구현 상태로 항상 0을 반환한다 (추후 구현 예정).
 */
int countImminent(void) {
    return 0;  /* TODO: 오늘 날짜와 expiry 비교 후 임박 개수 반환 */
}

/*
 * showMenu - 메인 메뉴 화면을 출력하고 사용자 입력을 기다린다.
 * 유통기한 임박 재료가 있으면 메뉴 항목 4번에 경고 문구를 표시한다.
 */
void showMenu(void) {
    int imm = countImminent();  /* 임박 재료 수 조회 */

    printLine();
    printf("        FridgeChef - 냉장고 재고 & 레시피 관리\n");
    printLine();
    printf("\n");
    printf("  데이터 불러오는 중...\n");
    printf("  재료 %d 로드 완료료.\n", itemCount);  /* 현재 로드된 재료 수 표시 */
    printf("\n");
    printf("  +------------------------------------------------------+\n");
    printf("  |                    메인 메뉴                         |\n");
    printf("  +------------------------------------------------------+\n");
    printf("  |  1.  재료 목록 보기                                  |\n");
    printf("  |  2.  재료 추가                                       |\n");
    printf("  |  3.  재료 삭제                                       |\n");

    /* 임박 재료가 1개 이상이면 경고 메시지를 함께 출력 */
    if (imm > 0)
        printf("  |  4.  유통기한 임박 확인     !! %d개 임박!             |\n", imm);
    else
        printf("  |  4.  유통기한 임박 확인                              |\n");

    printf("  |  5.  레시피 추천                                     |\n");
    printf("  |  0.  종료                                            |\n");
    printf("  +------------------------------------------------------+\n");
    printf("\n  선택 >> ");
}

/*
 * listItems - 현재 재고에 등록된 재료 목록을 출력한다.
 * TODO: inventory 배열을 순회하며 각 Item 정보를 표 형태로 출력한다.
 */
void listItems(void) {
    printf("\n  [재료 목록] \n");
    printf("  Enter 를 누르면 메인 메뉴로 돌아갑니다...");
    getchar(); getchar();  /* 첫 getchar()로 이전 입력 버퍼 비우기, 두 번째로 Enter 대기 */
}

/*
 * addItem - 사용자 입력을 받아 새 재료를 inventory 배열에 추가한다.
 * TODO: name, category, location, quantity, expiry 입력 받아 저장 후
 *       FILENAME 파일에 기록한다.
 */
void addItem(void) {
    printf("\n  [재료 추가] \n");
    printf("  Enter 를 누르면 메인 메뉴로 돌아갑니다...");
    getchar(); getchar();
}

/*
 * deleteItem - 사용자가 선택한 재료를 inventory 배열에서 제거한다.
 * TODO: 이름 또는 번호로 검색 후 해당 항목을 삭제하고 파일을 갱신한다.
 */
void deleteItem(void) {
    printf("\n  [재료 삭제] \n");
    printf("  Enter 를 누르면 메인 메뉴로 돌아갑니다...");
    getchar(); getchar();
}

/*
 * checkAlert - 유통기한이 IMMINENT일 이내로 남은 재료를 화면에 출력한다.
 * TODO: 오늘 날짜(time/localtime)와 각 Item.expiry를 비교하여
 *       임박 재료만 필터링해 표시한다.
 */
void checkAlert(void) {
    printf("\n  [유통기한 임박 확인] \n");
    printf("  Enter 를 누르면 메인 메뉴로 돌아갑니다...");
    getchar(); getchar();
}

/*
 * showRecipes - 현재 재고를 바탕으로 만들 수 있는 레시피를 추천한다.
 * TODO: recipes 배열의 각 레시피에 대해 ingredients가 모두 inventory에
 *       존재하는지 확인하고 가능한 레시피 목록을 출력한다.
 */
void showRecipes(void) {
    printf("\n  [레시피 추천]\n");
    printf("  Enter 를 누르면 메인 메뉴로 돌아갑니다...");
    getchar(); getchar();
}

/*
 * main - 프로그램 진입점.
 * Windows 환경에서 UTF-8 출력을 위해 콘솔 코드페이지를 설정하고,
 * 메인 루프에서 메뉴를 반복 출력하며 사용자 입력에 따라 각 기능을 호출한다.
 */
int main(void) {
    int  choice;   /* 사용자가 선택한 메뉴 번호 */
    char confirm;  /* 종료 확인 입력 ('y'/'n') */

#ifdef _WIN32
    /* Windows 콘솔에서 한글(UTF-8)이 깨지지 않도록 코드페이지 설정 */
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    while (1) {
        /* 매 루프마다 화면을 지우고 메뉴를 새로 출력 */
#ifdef _WIN32
        system("cls");    /* Windows: 화면 지우기 */
#else
        system("clear");  /* Linux/macOS: 화면 지우기 */
#endif
        showMenu();
        scanf("%d", &choice);  /* 메뉴 번호 입력 */

        switch (choice) {
            case 1: listItems();   break;  /* 재료 목록 보기 */
            case 2: addItem();     break;  /* 재료 추가 */
            case 3: deleteItem();  break;  /* 재료 삭제 */
            case 4: checkAlert();  break;  /* 유통기한 임박 확인 */
            case 5: showRecipes(); break;  /* 레시피 추천 */
            case 0:
                /* 종료 전 재확인 */
                printf("\n  정말 종료하시겠습니까? (y/n) >> ");
                scanf(" %c", &confirm);  /* 공백 앞에 ' '를 두어 개행 문자 스킵 */
                if (confirm == 'y' || confirm == 'Y') {
                    printLine();
                    printf("    FridgeChef 를 종료합니다. 감사합니다!\n");
                    printLine();
                    return 0;  /* 정상 종료 */
                }
                break;
            default:
                /* 유효하지 않은 번호 입력 시 안내 메시지 출력 */
                printf("\n  잘못된 입력입니다. 다시 선택하세요\n");
                printf("  Enter 를 누르면 메인 메뉴로 돌아갑니다...");
                getchar(); getchar();
                break;
        }
    }
    return 0;  /* 컴파일러 경고 방지용 (실제로는 도달하지 않음) */
}
