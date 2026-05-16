#define _CRT_SECURE_NO_WARNINGS  /* Visual Studio에서 구형 함수 경고 비활성화 */

#include <stdio.h>    /* 표준 입출력 (printf, scanf 등) */
#include <stdlib.h>   /* 표준 라이브러리 (system 등) */
#include <string.h>   /* 문자열 처리 함수 (strcmp, strcpy 등) */
#include <time.h>     /* 시간 관련 함수 (유통기한 비교) */

#ifdef _WIN32
#include <windows.h>  /* Windows 전용 API (콘솔 코드페이지 설정 등) */
#endif

/* ── 상수 정의 ───────────────────────────────────────────────── */
#define MAX_ITEMS    100
#define MAX_RECIPES  10
#define MAX_ING      5
#define IMMINENT     3
#define FILENAME     "inventory.txt"

/* ── 재료 구조체 ─────────────────────────────────────────────── */
typedef struct {
    char name[50];
    char category[20];
    char location[10];
    int  quantity;
    char expiry[11];   /* "YYYY-MM-DD" */
} Item;

/* ── 레시피 구조체 ───────────────────────────────────────────── */
typedef struct {
    char name[50];
    char ingredients[MAX_ING][50];
    int  ingCount;
} Recipe;

/* ── 전역 변수 ───────────────────────────────────────────────── */
Item   inventory[MAX_ITEMS];
int    itemCount   = 0;
Recipe recipes[MAX_RECIPES];
int    recipeCount = 0;

/* ── 함수 전방 선언 ──────────────────────────────────────────── */
void listItems(void);
void addItem(void);
void deleteItem(void);
void checkAlert(void);
void showRecipes(void);
void saveItems(void);
void loadItems(void);
int  daysUntilExpiry(const char *expiry);   /* [3주차] */
int  countImminent(void);                   /* [3주차] */

/* ── 유틸 ────────────────────────────────────────────────────── */
void printLine(void) {
    printf("================================================================\n");
}

static void flushInput(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

/* ════════════════════════════════════════════════════════════════
 * [3주차] 날짜 계산
 * ════════════════════════════════════════════════════════════════ */

/*
 * daysUntilExpiry - "YYYY-MM-DD" 문자열을 받아
 * 오늘 자정 기준으로 남은 일수(정수)를 반환한다.
 * 음수 = 이미 만료 / 파싱 실패 = 9999
 */
int daysUntilExpiry(const char *expiry) {
    struct tm exp_tm   = {0};
    struct tm today_tm;
    time_t    now, exp_time, today_midnight;
    double    diff;

    if (sscanf(expiry, "%d-%d-%d",
               &exp_tm.tm_year, &exp_tm.tm_mon, &exp_tm.tm_mday) != 3)
        return 9999;

    exp_tm.tm_year -= 1900;
    exp_tm.tm_mon  -= 1;
    exp_tm.tm_isdst = -1;
    exp_time = mktime(&exp_tm);

    now            = time(NULL);
    today_tm       = *localtime(&now);
    today_tm.tm_hour = 0;
    today_tm.tm_min  = 0;
    today_tm.tm_sec  = 0;
    today_midnight   = mktime(&today_tm);

    diff = difftime(exp_time, today_midnight);
    return (int)(diff / (60.0 * 60.0 * 24.0));
}

/*
 * countImminent - IMMINENT일 이내(만료 포함)인 재료 수를 반환한다.
 */
int countImminent(void) {
    int i, count = 0;
    for (i = 0; i < itemCount; i++)
        if (daysUntilExpiry(inventory[i].expiry) <= IMMINENT)
            count++;
    return count;
}

/* ════════════════════════════════════════════════════════════════
 * 메인 메뉴
 * ════════════════════════════════════════════════════════════════ */

void showMenu(void) {
    int imm = countImminent();  /* [3주차] 실제 날짜 계산 결과 반영 */

    printLine();
    printf("        FridgeChef - 냉장고 재고 & 레시피 관리\n");
    printLine();
    printf("\n");
    printf("  재료 %d개 로드 완료.\n", itemCount);
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

/* ════════════════════════════════════════════════════════════════
 * 파일 입출력 (2주차)
 * ════════════════════════════════════════════════════════════════ */

void saveItems(void) {
    int i;
    FILE *fp = fopen(FILENAME, "w");
    if (!fp) { printf("  [오류] 파일 저장에 실패했습니다.\n"); return; }
    for (i = 0; i < itemCount; i++)
        fprintf(fp, "%s|%s|%s|%d|%s\n",
                inventory[i].name, inventory[i].category,
                inventory[i].location, inventory[i].quantity,
                inventory[i].expiry);
    fclose(fp);
}

void loadItems(void) {
    FILE *fp = fopen(FILENAME, "r");
    if (!fp) return;
    itemCount = 0;
    while (itemCount < MAX_ITEMS) {
        Item it;
        if (fscanf(fp, "%49[^|]|%19[^|]|%9[^|]|%d|%10[^\n]\n",
                   it.name, it.category, it.location,
                   &it.quantity, it.expiry) == 5)
            inventory[itemCount++] = it;
        else
            break;
    }
    fclose(fp);
}

/* ════════════════════════════════════════════════════════════════
 * 재료 목록 (2주차 + 3주차 D-day 컬럼 추가)
 * ════════════════════════════════════════════════════════════════ */

void listItems(void) {
    int i;

    printf("\n");
    printLine();
    printf("  [재료 목록] - 총 %d개\n", itemCount);
    printLine();

    if (itemCount == 0) {
        printf("  등록된 재료가 없습니다.\n");
        printf("  메뉴 2번으로 재료를 추가해 보세요!\n");
    } else {
        /* [3주차] D-day 컬럼 추가 */
        printf("  +----+------------------+----------+------+------+------------+----------+\n");
        printf("  | No |      이름        |   분류   | 위치 | 수량 |  유통기한  |  D-day   |\n");
        printf("  +----+------------------+----------+------+------+------------+----------+\n");
        for (i = 0; i < itemCount; i++) {
            int  days = daysUntilExpiry(inventory[i].expiry);
            char dstr[16];
            if      (days == 9999) snprintf(dstr, sizeof(dstr), "  -     ");
            else if (days <  0   ) snprintf(dstr, sizeof(dstr), " 만료됨 ");
            else if (days == 0   ) snprintf(dstr, sizeof(dstr), "오늘만료");
            else if (days <= IMMINENT) snprintf(dstr, sizeof(dstr), "  D-%-4d", days);
            else                   snprintf(dstr, sizeof(dstr), "        ");

            printf("  |%3d | %-16s | %-8s | %-4s | %3d  | %-10s | %-8s |\n",
                   i + 1,
                   inventory[i].name, inventory[i].category,
                   inventory[i].location, inventory[i].quantity,
                   inventory[i].expiry, dstr);
        }
        printf("  +----+------------------+----------+------+------+------------+----------+\n");
        printf("  ※ D-day 칸이 채워진 항목은 유통기한 %d일 이내 재료입니다.\n", IMMINENT);
    }

    printf("\n  Enter 를 누르면 메인 메뉴로 돌아갑니다...");
    flushInput();
    getchar();
}

/* ════════════════════════════════════════════════════════════════
 * 재료 추가 (2주차 + 3주차 즉시 임박 경고 추가)
 * ════════════════════════════════════════════════════════════════ */

void addItem(void) {
    Item newItem;
    int  catChoice, locChoice;

    printf("\n");
    printLine();
    printf("  [재료 추가]\n");
    printLine();

    if (itemCount >= MAX_ITEMS) {
        printf("  재고가 가득 찼습니다. (최대 %d개)\n", MAX_ITEMS);
        printf("  Enter 를 누르면 메인 메뉴로 돌아갑니다...");
        flushInput(); getchar(); return;
    }

    flushInput();

    /* 1. 재료 이름 */
    printf("  재료 이름              : ");
    fgets(newItem.name, sizeof(newItem.name), stdin);
    newItem.name[strcspn(newItem.name, "\n")] = '\0';
    if (strlen(newItem.name) == 0) {
        printf("  이름을 입력하지 않았습니다. 취소합니다.\n");
        printf("  Enter 를 누르면 메인 메뉴로 돌아갑니다...");
        getchar(); return;
    }

    /* 2. 분류 */
    printf("\n  분류 선택\n");
    printf("    1. 채소    2. 과일    3. 육류\n");
    printf("    4. 해산물  5. 유제품  6. 기타\n");
    printf("  선택 >> ");
    scanf("%d", &catChoice);
    switch (catChoice) {
        case 1: strcpy(newItem.category, "채소");   break;
        case 2: strcpy(newItem.category, "과일");   break;
        case 3: strcpy(newItem.category, "육류");   break;
        case 4: strcpy(newItem.category, "해산물"); break;
        case 5: strcpy(newItem.category, "유제품"); break;
        default: strcpy(newItem.category, "기타");  break;
    }

    /* 3. 보관 위치 */
    printf("\n  보관 위치 선택\n");
    printf("    1. 냉장    2. 냉동    3. 실온\n");
    printf("  선택 >> ");
    scanf("%d", &locChoice);
    switch (locChoice) {
        case 1: strcpy(newItem.location, "냉장"); break;
        case 2: strcpy(newItem.location, "냉동"); break;
        default: strcpy(newItem.location, "실온"); break;
    }

    /* 4. 수량 */
    printf("\n  수량 (개)              : ");
    scanf("%d", &newItem.quantity);
    if (newItem.quantity <= 0) newItem.quantity = 1;

    /* 5. 유통기한 */
    flushInput();
    printf("  유통기한 (YYYY-MM-DD)  : ");
    fgets(newItem.expiry, sizeof(newItem.expiry), stdin);
    newItem.expiry[strcspn(newItem.expiry, "\n")] = '\0';
    if (strlen(newItem.expiry) == 0) strcpy(newItem.expiry, "미정");

    inventory[itemCount++] = newItem;
    saveItems();

    /* [3주차] 추가 직후 임박 여부 즉시 경고 */
    {
        int days = daysUntilExpiry(newItem.expiry);
        printf("\n");
        printLine();
        printf("  [%s] 재료가 추가되었습니다!\n", newItem.name);
        printf("    분류: %s  |  위치: %s  |  수량: %d개  |  유통기한: %s\n",
               newItem.category, newItem.location,
               newItem.quantity, newItem.expiry);
        if (days < 0)
            printf("  !! 이미 유통기한이 지난 재료입니다!\n");
        else if (days <= IMMINENT)
            printf("  !! 유통기한이 %d일 남았습니다. 빨리 사용하세요!\n", days);
        printf("    현재 등록된 재료: 총 %d개\n", itemCount);
        printLine();
    }

    printf("  Enter 를 누르면 메인 메뉴로 돌아갑니다...");
    getchar();
}

/* ════════════════════════════════════════════════════════════════
 * 재료 삭제 (2주차)
 * ════════════════════════════════════════════════════════════════ */

void deleteItem(void) {
    int i, choice;

    printf("\n");
    printLine();
    printf("  [재료 삭제]\n");
    printLine();

    if (itemCount == 0) {
        printf("  삭제할 재료가 없습니다.\n");
        printf("\n  Enter 를 누르면 메인 메뉴로 돌아갑니다...");
        flushInput(); getchar(); return;
    }

    printf("  번호  재료명\n");
    printf("  ----  ----------------\n");
    for (i = 0; i < itemCount; i++)
        printf("  %3d.  %s\n", i + 1, inventory[i].name);
    printf("\n");

    printf("  삭제할 재료 번호 (0: 취소) >> ");
    scanf("%d", &choice);

    if (choice <= 0 || choice > itemCount) {
        printf("  취소하거나 잘못된 번호입니다.\n");
    } else {
        char deleted[50];
        strcpy(deleted, inventory[choice - 1].name);
        for (i = choice - 1; i < itemCount - 1; i++)
            inventory[i] = inventory[i + 1];
        itemCount--;
        saveItems();
        printf("  [%s] 재료가 삭제되었습니다. (남은 재료: %d개)\n",
               deleted, itemCount);
    }

    printf("\n  Enter 를 누르면 메인 메뉴로 돌아갑니다...");
    flushInput(); getchar();
}

/* ════════════════════════════════════════════════════════════════
 * [3주차] 유통기한 임박 확인
 * ════════════════════════════════════════════════════════════════ */

/*
 * checkAlert - IMMINENT일 이내(만료 포함)인 재료만 필터링해 출력한다.
 * 만료: "만료됨" / 당일: "오늘만료" / 임박: "D-N"
 */
void checkAlert(void) {
    int i, found = 0;

    printf("\n");
    printLine();
    printf("  [유통기한 임박 확인]  기준: %d일 이내 또는 만료\n", IMMINENT);
    printLine();

    printf("  +----+------------------+------+------+------------+-----------+\n");
    printf("  | No |      이름        | 위치 | 수량 |  유통기한  |   상태    |\n");
    printf("  +----+------------------+------+------+------------+-----------+\n");

    for (i = 0; i < itemCount; i++) {
        int  days = daysUntilExpiry(inventory[i].expiry);
        char status[16];

        if (days > IMMINENT) continue;  /* 여유 있으면 건너뜀 */

        found++;
        if      (days <  0) snprintf(status, sizeof(status), "만료됨");
        else if (days == 0) snprintf(status, sizeof(status), "오늘만료");
        else                snprintf(status, sizeof(status), "D-%d", days);

        printf("  |%3d | %-16s | %-4s | %3d  | %-10s | %-9s |\n",
               found,
               inventory[i].name,
               inventory[i].location,
               inventory[i].quantity,
               inventory[i].expiry,
               status);
    }

    printf("  +----+------------------+------+------+------------+-----------+\n");

    if (found == 0)
        printf("  임박한 재료가 없습니다. 모든 재료가 신선합니다!\n");
    else
        printf("  총 %d개의 재료를 확인하세요.\n", found);

    printLine();
    printf("  Enter 를 누르면 메인 메뉴로 돌아갑니다...");
    flushInput(); getchar();
}

/* ════════════════════════════════════════════════════════════════
 * 레시피 추천 (4주차 예정)
 * ════════════════════════════════════════════════════════════════ */

void showRecipes(void) {
    printf("\n");
    printLine();
    printf("  [레시피 추천]\n");
    printLine();
    printf("  레시피 추천 기능은 4주차에 구현될 예정입니다.\n");
    printLine();
    printf("  Enter 를 누르면 메인 메뉴로 돌아갑니다...");
    flushInput(); getchar();
}

/* ════════════════════════════════════════════════════════════════
 * main
 * ════════════════════════════════════════════════════════════════ */

int main(void) {
    int  choice;
    char confirm;

#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    loadItems();

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
                printf("\n  잘못된 입력입니다. 다시 선택하세요.\n");
                printf("  Enter 를 누르면 메인 메뉴로 돌아갑니다...");
                flushInput(); getchar();
                break;
        }
    }
    return 0;
}
