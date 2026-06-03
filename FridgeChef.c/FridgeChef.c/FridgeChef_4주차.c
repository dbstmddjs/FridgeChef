/*
 * FridgeChef - 4주차
 * 팀원: 2022243112 윤승언 / 2024243058 김민찬 / 2025243068 안민
 *
 * [레시피 DB 방식]
 *  - recipes.db 파일을 직접 편집해서 레시피 추가/수정/삭제
 *  - 프로그램 시작 시 recipes.db 자동 로드
 *  - 보유 재료와 매칭률 계산 후 높은 순 추천
 *
 * [recipes.db 형식]
 *  # 으로 시작하는 줄은 주석 (무시됨)
 *  레시피이름|재료1|재료2|재료3|...
 *  예) 된장찌개|두부|감자|애호박|된장|대파|마늘
 */

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#ifdef _WIN32
#include <windows.h>
#endif

/* ── 상수 ───────────────────────────────────────────────────────── */
#define MAX_ITEMS    100
#define MAX_RECIPES  200
#define MAX_ING      15
#define IMMINENT     3
#define FILENAME     "inventory.txt"
#define RECIPE_DB    "recipes.db"

/* ── 재료 구조체 ─────────────────────────────────────────────────── */
typedef struct {
    char name[50];
    char category[20];
    char location[10];
    int  quantity;
    char expiry[11];
} Item;

/* ── 레시피 구조체 ───────────────────────────────────────────────── */
typedef struct {
    char name[50];
    char ingredients[MAX_ING][50];
    int  ingCount;
} Recipe;

/* ── 전역 변수 ───────────────────────────────────────────────────── */
Item   inventory[MAX_ITEMS];
int    itemCount   = 0;
Recipe recipes[MAX_RECIPES];
int    recipeCount = 0;

/* ── 함수 전방 선언 ──────────────────────────────────────────────── */
void printLine(void);
static void flushInput(void);
int  daysUntilExpiry(const char *expiry);
int  countImminent(void);
void loadRecipes(void);
void showMenu(void);
void listItems(void);
void searchItem(void);
void sortItemsByExpiry(void);
void addItem(void);
void editItem(void);
void deleteItem(void);
void checkAlert(void);
void showRecipes(void);
void saveItems(void);
void loadItems(void);

/* ════════════════════════════════════════════════════════════════════
 * 유틸리티
 * ════════════════════════════════════════════════════════════════════ */

void printLine(void) {
    printf("================================================================\n");
}

static void flushInput(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

/* ════════════════════════════════════════════════════════════════════
 * 날짜 계산
 * ════════════════════════════════════════════════════════════════════ */

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

    now              = time(NULL);
    today_tm         = *localtime(&now);
    today_tm.tm_hour = 0;
    today_tm.tm_min  = 0;
    today_tm.tm_sec  = 0;
    today_midnight   = mktime(&today_tm);

    diff = difftime(exp_time, today_midnight);
    return (int)(diff / (60.0 * 60.0 * 24.0));
}

int countImminent(void) {
    int i, count = 0;
    for (i = 0; i < itemCount; i++)
        if (daysUntilExpiry(inventory[i].expiry) <= IMMINENT)
            count++;
    return count;
}

/* ════════════════════════════════════════════════════════════════════
 * 파일 입출력 — 재료
 * ════════════════════════════════════════════════════════════════════ */

void saveItems(void) {
    int i;
    FILE *fp = fopen(FILENAME, "w");
    if (!fp) { printf("  [오류] 재료 파일 저장 실패.\n"); return; }
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

/* ════════════════════════════════════════════════════════════════════
 * 레시피 DB 로드
 * recipes.db 형식:
 *   된장찌개|두부|감자|애호박|된장|대파|마늘
 *   김치찌개|김치|돼지고기|두부|대파|마늘
 * ════════════════════════════════════════════════════════════════════ */

void loadRecipes(void) {
    FILE *fp;
    char  line[512];
    char *token;
    int   lineNum = 0;

    recipeCount = 0;

    fp = fopen(RECIPE_DB, "r");
    if (!fp) {
        printf("  [경고] recipes.db 파일을 찾을 수 없습니다.\n");
        printf("         실행 파일과 같은 폴더에 recipes.db 를 만들어 주세요.\n");
        return;
    }

    while (fgets(line, sizeof(line), fp)) {
        lineNum++;

        /* 개행 제거 */
        line[strcspn(line, "\r\n")] = '\0';

        /* 빈 줄 또는 주석 줄 건너뜀 */
        if (line[0] == '\0' || line[0] == '#') continue;

        /* 최대 레시피 수 초과 */
        if (recipeCount >= MAX_RECIPES) break;

        /* 첫 번째 토큰 = 레시피 이름 */
        token = strtok(line, "|");
        if (!token) continue;

        strncpy(recipes[recipeCount].name, token, 49);
        recipes[recipeCount].name[49] = '\0';
        recipes[recipeCount].ingCount = 0;

        /* 나머지 토큰 = 재료들 */
        while ((token = strtok(NULL, "|")) != NULL) {
            if (recipes[recipeCount].ingCount >= MAX_ING) break;
            strncpy(recipes[recipeCount].ingredients[recipes[recipeCount].ingCount],
                    token, 49);
            recipes[recipeCount].ingredients[recipes[recipeCount].ingCount][49] = '\0';
            recipes[recipeCount].ingCount++;
        }

        /* 재료가 1개 이상인 경우만 등록 */
        if (recipes[recipeCount].ingCount > 0)
            recipeCount++;
    }

    fclose(fp);
    printf("  레시피 DB 로드 완료: %d개\n", recipeCount);
}

/* ════════════════════════════════════════════════════════════════════
 * 메인 메뉴
 * ════════════════════════════════════════════════════════════════════ */

void showMenu(void) {
    int imm = countImminent();

    printLine();
    printf("        FridgeChef - 냉장고 재고 & 레시피 관리\n");
    printLine();
    printf("\n");
    printf("  재료 %d개 등록 | 레시피 DB %d개 로드\n", itemCount, recipeCount);
    printf("\n");
    printf("  +------------------------------------------------------+\n");
    printf("  |              [ 재료 관리 ]                           |\n");
    printf("  +------------------------------------------------------+\n");
    printf("  |  1.  재료 목록 보기                                  |\n");
    printf("  |  2.  재료 추가                                       |\n");
    printf("  |  3.  재료 삭제                                       |\n");
    printf("  |  4.  재료 수정                                       |\n");
    printf("  |  5.  재료 검색                                       |\n");
    printf("  |  6.  유통기한 순 정렬                                |\n");
    if (imm > 0)
        printf("  |  7.  유통기한 임박 확인     !! %2d개 임박!           |\n", imm);
    else
        printf("  |  7.  유통기한 임박 확인                              |\n");
    printf("  +------------------------------------------------------+\n");
    printf("  |              [ 레시피 추천 ]                         |\n");
    printf("  +------------------------------------------------------+\n");
    printf("  |  8.  레시피 추천 (보유 재료 매칭률 순)               |\n");
    printf("  +------------------------------------------------------+\n");
    printf("  |  0.  종료                                            |\n");
    printf("  +------------------------------------------------------+\n");
    printf("\n  선택 >> ");
}

/* ════════════════════════════════════════════════════════════════════
 * 재료 목록
 * ════════════════════════════════════════════════════════════════════ */

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

/* ════════════════════════════════════════════════════════════════════
 * 재료 검색
 * ════════════════════════════════════════════════════════════════════ */

void searchItem(void) {
    char keyword[50];
    int  i, found = 0;

    printf("\n");
    printLine();
    printf("  [재료 검색]\n");
    printLine();

    flushInput();
    printf("  검색할 재료 이름 (일부만 입력 가능) : ");
    fgets(keyword, sizeof(keyword), stdin);
    keyword[strcspn(keyword, "\n")] = '\0';

    if (strlen(keyword) == 0) {
        printf("  검색어를 입력하지 않았습니다.\n");
        printf("\n  Enter 를 누르면 메인 메뉴로 돌아갑니다...");
        getchar();
        return;
    }

    printf("\n");
    printf("  [검색 결과: \"%s\"]\n", keyword);
    printf("  +----+------------------+----------+------+------+------------+----------+\n");
    printf("  | No |      이름        |   분류   | 위치 | 수량 |  유통기한  |  D-day   |\n");
    printf("  +----+------------------+----------+------+------+------------+----------+\n");

    for (i = 0; i < itemCount; i++) {
        char haystack[50], needle[50];
        int k;
        strcpy(haystack, inventory[i].name);
        strcpy(needle,   keyword);
        for (k = 0; haystack[k]; k++) haystack[k] = (char)tolower((unsigned char)haystack[k]);
        for (k = 0; needle[k];   k++) needle[k]   = (char)tolower((unsigned char)needle[k]);

        if (strstr(haystack, needle) == NULL) continue;

        found++;
        {
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
    }

    printf("  +----+------------------+----------+------+------+------------+----------+\n");

    if (found == 0)
        printf("  검색 결과가 없습니다.\n");
    else
        printf("  검색된 재료: %d개\n", found);

    printf("\n  Enter 를 누르면 메인 메뉴로 돌아갑니다...");
    flushInput();
    getchar();
}

/* ════════════════════════════════════════════════════════════════════
 * 유통기한 오름차순 정렬
 * ════════════════════════════════════════════════════════════════════ */

void sortItemsByExpiry(void) {
    int i, j;
    Item tmp;

    if (itemCount == 0) {
        printf("\n  등록된 재료가 없습니다.\n");
        printf("\n  Enter 를 누르면 메인 메뉴로 돌아갑니다...");
        flushInput(); getchar(); return;
    }

    for (i = 0; i < itemCount - 1; i++) {
        for (j = 0; j < itemCount - 1 - i; j++) {
            int a_mijeong = (strcmp(inventory[j].expiry,   "미정") == 0);
            int b_mijeong = (strcmp(inventory[j+1].expiry, "미정") == 0);
            int swap = 0;
            if      ( a_mijeong && !b_mijeong) swap = 1;
            else if (!a_mijeong && !b_mijeong)
                swap = (strcmp(inventory[j].expiry, inventory[j+1].expiry) > 0);
            if (swap) {
                tmp            = inventory[j];
                inventory[j]   = inventory[j+1];
                inventory[j+1] = tmp;
            }
        }
    }

    saveItems();

    printf("\n");
    printLine();
    printf("  [유통기한 순 정렬 완료] — 가까운 날짜부터 표시됩니다.\n");
    printLine();
    listItems();
}

/* ════════════════════════════════════════════════════════════════════
 * 재료 추가
 * ════════════════════════════════════════════════════════════════════ */

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

    printf("  재료 이름              : ");
    fgets(newItem.name, sizeof(newItem.name), stdin);
    newItem.name[strcspn(newItem.name, "\n")] = '\0';
    if (strlen(newItem.name) == 0) {
        printf("  이름을 입력하지 않았습니다. 취소합니다.\n");
        printf("  Enter 를 누르면 메인 메뉴로 돌아갑니다...");
        getchar(); return;
    }

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

    printf("\n  보관 위치 선택\n");
    printf("    1. 냉장    2. 냉동    3. 실온\n");
    printf("  선택 >> ");
    scanf("%d", &locChoice);
    switch (locChoice) {
        case 1: strcpy(newItem.location, "냉장"); break;
        case 2: strcpy(newItem.location, "냉동"); break;
        default: strcpy(newItem.location, "실온"); break;
    }

    printf("\n  수량 (개)              : ");
    scanf("%d", &newItem.quantity);
    if (newItem.quantity <= 0) newItem.quantity = 1;

    flushInput();
    printf("  유통기한 (YYYY-MM-DD)  : ");
    fgets(newItem.expiry, sizeof(newItem.expiry), stdin);
    newItem.expiry[strcspn(newItem.expiry, "\n")] = '\0';
    if (strlen(newItem.expiry) == 0) strcpy(newItem.expiry, "미정");

    inventory[itemCount++] = newItem;
    saveItems();

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

/* ════════════════════════════════════════════════════════════════════
 * 재료 수정
 * ════════════════════════════════════════════════════════════════════ */

void editItem(void) {
    int i, choice, field;

    printf("\n");
    printLine();
    printf("  [재료 수정]\n");
    printLine();

    if (itemCount == 0) {
        printf("  수정할 재료가 없습니다.\n");
        printf("\n  Enter 를 누르면 메인 메뉴로 돌아갑니다...");
        flushInput(); getchar(); return;
    }

    printf("  번호  재료명\n");
    printf("  ----  ----------------\n");
    for (i = 0; i < itemCount; i++)
        printf("  %3d.  %s\n", i + 1, inventory[i].name);
    printf("\n");

    printf("  수정할 재료 번호 (0: 취소) >> ");
    scanf("%d", &choice);

    if (choice <= 0 || choice > itemCount) {
        printf("  취소하거나 잘못된 번호입니다.\n");
        printf("\n  Enter 를 누르면 메인 메뉴로 돌아갑니다...");
        flushInput(); getchar(); return;
    }

    printf("\n  [%s] 수정 항목 선택\n", inventory[choice-1].name);
    printf("    1. 수량 변경  (현재: %d개)\n", inventory[choice-1].quantity);
    printf("    2. 유통기한 변경  (현재: %s)\n", inventory[choice-1].expiry);
    printf("    0. 취소\n");
    printf("  선택 >> ");
    scanf("%d", &field);
    flushInput();

    switch (field) {
        case 1:
            printf("  새 수량 (개) : ");
            scanf("%d", &inventory[choice-1].quantity);
            if (inventory[choice-1].quantity <= 0)
                inventory[choice-1].quantity = 1;
            saveItems();
            printf("  수량이 %d개로 변경되었습니다.\n", inventory[choice-1].quantity);
            flushInput();
            break;
        case 2:
            printf("  새 유통기한 (YYYY-MM-DD) : ");
            fgets(inventory[choice-1].expiry, sizeof(inventory[choice-1].expiry), stdin);
            inventory[choice-1].expiry[strcspn(inventory[choice-1].expiry, "\n")] = '\0';
            if (strlen(inventory[choice-1].expiry) == 0)
                strcpy(inventory[choice-1].expiry, "미정");
            saveItems();
            printf("  유통기한이 [%s]으로 변경되었습니다.\n", inventory[choice-1].expiry);
            break;
        default:
            printf("  취소합니다.\n");
            break;
    }

    printf("\n  Enter 를 누르면 메인 메뉴로 돌아갑니다...");
    flushInput();
    getchar();
}

/* ════════════════════════════════════════════════════════════════════
 * 재료 삭제
 * ════════════════════════════════════════════════════════════════════ */

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

/* ════════════════════════════════════════════════════════════════════
 * 유통기한 임박 확인
 * ════════════════════════════════════════════════════════════════════ */

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

        if (days > IMMINENT) continue;

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

/* ════════════════════════════════════════════════════════════════════
 * 레시피 추천 — 매칭률 높은 순 전체 출력
 * ════════════════════════════════════════════════════════════════════ */

static int haveItem(const char *ingName) {
    int i, k;
    char hay[50], needle[50];
    strcpy(needle, ingName);
    for (k = 0; needle[k]; k++) needle[k] = (char)tolower((unsigned char)needle[k]);
    for (i = 0; i < itemCount; i++) {
        strcpy(hay, inventory[i].name);
        for (k = 0; hay[k]; k++) hay[k] = (char)tolower((unsigned char)hay[k]);
        if (strstr(hay, needle))
            return 1;
    }
    return 0;
}

void showRecipes(void) {
    int i, j;

    typedef struct { int idx; float score; int matched; } MatchResult;
    MatchResult results[MAX_RECIPES];
    int         rcount = 0;
    MatchResult tmp_r;
    int         shown  = 0;

    printf("\n");
    printLine();
    printf("  [레시피 추천] — 보유 재료 매칭률 순\n");
    printLine();

    if (recipeCount == 0) {
        printf("  recipes.db 에 레시피가 없습니다.\n");
        printf("  recipes.db 파일을 확인해 주세요.\n");
        printf("\n  Enter 를 누르면 메인 메뉴로 돌아갑니다...");
        flushInput(); getchar(); return;
    }

    if (itemCount == 0) {
        printf("  재료가 없습니다. 먼저 재료를 추가해 주세요! (메뉴 2번)\n");
        printf("\n  Enter 를 누르면 메인 메뉴로 돌아갑니다...");
        flushInput(); getchar(); return;
    }

    /* 1단계: 매칭률 계산 */
    for (i = 0; i < recipeCount; i++) {
        int   matched = 0;
        float score;

        for (j = 0; j < recipes[i].ingCount; j++)
            if (haveItem(recipes[i].ingredients[j]))
                matched++;

        score = (recipes[i].ingCount > 0)
                ? (float)matched / recipes[i].ingCount * 100.0f
                : 0.0f;

        results[rcount].idx     = i;
        results[rcount].score   = score;
        results[rcount].matched = matched;
        rcount++;
    }

    /* 2단계: 버블 정렬 (매칭률 내림차순) */
    for (i = 0; i < rcount - 1; i++) {
        for (j = 0; j < rcount - 1 - i; j++) {
            if (results[j].score < results[j+1].score) {
                tmp_r        = results[j];
                results[j]   = results[j+1];
                results[j+1] = tmp_r;
            }
        }
    }

    /* 3단계: 출력 (매칭 1개 이상만) */
    for (i = 0; i < rcount; i++) {
        int ri = results[i].idx;

        if (results[i].matched == 0) break;

        shown++;
        printf("  %2d위. %-20s  %5.1f%%  (%d / %d 재료 보유)\n",
               shown,
               recipes[ri].name,
               results[i].score,
               results[i].matched,
               recipes[ri].ingCount);

        printf("        재료: ");
        for (j = 0; j < recipes[ri].ingCount; j++) {
            int have = haveItem(recipes[ri].ingredients[j]);
            if (!have) printf("[없음]");
            printf("%s", recipes[ri].ingredients[j]);
            if (j < recipes[ri].ingCount - 1) printf(", ");
        }
        printf("\n");

        if (i < rcount - 1 && results[i+1].matched > 0)
            printf("  -----------------------------------------------\n");
    }

    if (shown == 0) {
        printf("  보유 재료와 매칭되는 레시피가 없습니다.\n");
        printf("  재료를 더 추가해 보세요!\n");
    } else {
        printf("\n");
        printLine();
        printf("  총 %d개 레시피 추천  |  매칭 재료 0개인 레시피 %d개 숨김\n",
               shown, rcount - shown);
    }

    printLine();
    printf("  Enter 를 누르면 메인 메뉴로 돌아갑니다...");
    flushInput(); getchar();
}

/* ════════════════════════════════════════════════════════════════════
 * main
 * ════════════════════════════════════════════════════════════════════ */

int main(void) {
    int  choice;
    char confirm;

#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    loadRecipes();   /* recipes.db 로드 */
    loadItems();     /* inventory.txt 로드 */

    while (1) {
#ifdef _WIN32
        system("cls");
#else
        system("clear");
#endif
        showMenu();
        scanf("%d", &choice);

        switch (choice) {
            case 1: listItems();         break;
            case 2: addItem();           break;
            case 3: deleteItem();        break;
            case 4: editItem();          break;
            case 5: searchItem();        break;
            case 6: sortItemsByExpiry(); break;
            case 7: checkAlert();        break;
            case 8: showRecipes();       break;
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
