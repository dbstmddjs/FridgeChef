/*
 * FridgeChef - 4주차
 * 팀원: 2022243112 윤승언 / 2024243058 김민찬 / 2025243068 안민
 *
 * [4주차 구현 목록]
 *  - 레시피 DB 파일 연결 (3주차 DB 명세서 기반)
 *    · recipes.db   : 레시피아이디 | 레시피이름 | 재료수
 *    · recipe_ing.db: 재료아이디 | 레시피아이디 | 재료이름
 *  - 레시피 추가 / 목록 출력 / 삭제
 *  - 레시피 추천 (보유 재료 매칭률 계산)
 *  - 임박 재료 우선 추천 모드
 *  - 재료 검색 (이름 부분 일치)
 *  - 재료 정렬 (유통기한 오름차순)
 *  - 재료 수정 (수량 / 유통기한 변경)
 *  - 모든 기능 메인 루프 연결 완료
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
#define MAX_ITEMS      100
#define MAX_RECIPES    50   /* [4주차] 10 → 50 확장 */
#define MAX_ING        10   /* [4주차] 5  → 10 확장 */
#define IMMINENT       3
#define FILENAME       "inventory.txt"
#define RECIPE_DB      "recipes.db"      /* [4주차] 레시피 DB 파일 */
#define RECIPE_ING_DB  "recipe_ing.db"   /* [4주차] 레시피재료 DB 파일 */

/* ── 재료 구조체 ─────────────────────────────────────────────────── */
typedef struct {
    char name[50];
    char category[20];
    char location[10];
    int  quantity;
    char expiry[11];   /* "YYYY-MM-DD" */
} Item;

/*
 * [4주차] DB 명세서 반영 레시피 구조체
 *
 * 레시피 릴레이션  : recipeId (PK, AUTO_INCREMENT) | name | ingCount
 * 레시피재료 릴레이션: ingId (PK, AUTO_INCREMENT) | recipeId (FK) | ingName
 *
 * C 구조체로 두 릴레이션을 하나로 표현 —
 * ingId 배열 인덱스가 재료아이디 역할,  recipeId 필드가 외래키 역할.
 */
typedef struct {
    int  recipeId;                   /* 레시피아이디 (PK, AUTO INCREMENT) */
    char name[50];                   /* 레시피이름 */
    char ingredients[MAX_ING][50];   /* 재료이름 배열 (레시피재료 릴레이션) */
    int  ingCount;                   /* 재료수 */
} Recipe;

/* ── 전역 변수 ───────────────────────────────────────────────────── */
Item   inventory[MAX_ITEMS];
int    itemCount   = 0;
Recipe recipes[MAX_RECIPES];
int    recipeCount = 0;
static int nextRecipeId = 1;   /* AUTO INCREMENT 시뮬레이션 */

/* ── 함수 전방 선언 ──────────────────────────────────────────────── */
/* 유틸 */
void printLine(void);
void printDoubleLine(void);
static void flushInput(void);

/* 날짜 */
int  daysUntilExpiry(const char *expiry);
int  countImminent(void);

/* 재료 */
void showMenu(void);
void listItems(void);
void searchItem(void);       /* [4주차] 재료 검색 */
void sortItemsByExpiry(void);/* [4주차] 유통기한 정렬 */
void addItem(void);
void editItem(void);         /* [4주차] 재료 수정 */
void deleteItem(void);
void checkAlert(void);

/* 레시피 */
void saveRecipes(void);      /* [4주차] 레시피 DB 저장 */
void loadRecipes(void);      /* [4주차] 레시피 DB 로드 */
void addRecipe(void);        /* [4주차] 레시피 추가 */
void listRecipes(void);      /* [4주차] 레시피 목록 */
void deleteRecipe(void);     /* [4주차] 레시피 삭제 */
void showRecipes(void);      /* [4주차] 레시피 추천 (매칭률) */
void recipeMenu(void);       /* [4주차] 레시피 서브메뉴 */

/* 파일 입출력 */
void saveItems(void);
void loadItems(void);

/* ════════════════════════════════════════════════════════════════════
 * 유틸리티
 * ════════════════════════════════════════════════════════════════════ */

void printLine(void) {
    printf("================================================================\n");
}

void printDoubleLine(void) {
    printf("################################################################\n");
}

static void flushInput(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

/* ════════════════════════════════════════════════════════════════════
 * 날짜 계산 (3주차)
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

    now            = time(NULL);
    today_tm       = *localtime(&now);
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
 * 파일 입출력 - 재료 (3주차)
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
 * [4주차] 파일 입출력 - 레시피 DB
 *
 * 저장 형식 (recipes.db):
 *   recipeId|레시피이름|재료수\n
 *
 * 저장 형식 (recipe_ing.db):
 *   recipeId|재료이름\n
 *   (레시피재료 릴레이션 — 한 레시피에 ingCount개 행)
 *
 * 3주차 DB 명세서:
 *   레시피 테이블     : 레시피아이디(PK) | 레시피이름 | 재료수
 *   레시피재료 테이블 : 재료아이디(PK) | 레시피아이디(FK) | 재료이름
 * ════════════════════════════════════════════════════════════════════ */

void saveRecipes(void) {
    int i, j;
    FILE *frec, *fing;

    /* recipes.db — 레시피 테이블 */
    frec = fopen(RECIPE_DB, "w");
    if (!frec) { printf("  [오류] 레시피 DB 저장 실패.\n"); return; }

    /* recipe_ing.db — 레시피재료 테이블 */
    fing = fopen(RECIPE_ING_DB, "w");
    if (!fing) {
        fclose(frec);
        printf("  [오류] 레시피재료 DB 저장 실패.\n");
        return;
    }

    /* nextRecipeId 헤더 줄 저장 */
    fprintf(frec, "#nextId=%d\n", nextRecipeId);

    for (i = 0; i < recipeCount; i++) {
        /* 레시피 테이블 한 행 */
        fprintf(frec, "%d|%s|%d\n",
                recipes[i].recipeId,
                recipes[i].name,
                recipes[i].ingCount);

        /* 레시피재료 테이블 — ingCount개 행 (재료아이디는 행 인덱스 j) */
        for (j = 0; j < recipes[i].ingCount; j++)
            fprintf(fing, "%d|%d|%s\n",
                    j + 1,                   /* 재료아이디 (AUTO INCREMENT) */
                    recipes[i].recipeId,     /* 레시피아이디 (FK) */
                    recipes[i].ingredients[j]);
    }

    fclose(frec);
    fclose(fing);
}

void loadRecipes(void) {
    int i;
    char line[256];
    FILE *frec, *fing;

    /* ── 1단계: recipes.db 로드 ── */
    frec = fopen(RECIPE_DB, "r");
    if (!frec) return;  /* 첫 실행 시 파일 없음 — 정상 */

    recipeCount  = 0;
    nextRecipeId = 1;

    while (fgets(line, sizeof(line), frec)) {
        /* 헤더 줄 파싱 */
        if (line[0] == '#') {
            sscanf(line, "#nextId=%d", &nextRecipeId);
            continue;
        }
        if (recipeCount >= MAX_RECIPES) break;

        {
            int  rid, ing;
            char rname[50];
            if (sscanf(line, "%d|%49[^|]|%d", &rid, rname, &ing) == 3) {
                recipes[recipeCount].recipeId = rid;
                strcpy(recipes[recipeCount].name, rname);
                recipes[recipeCount].ingCount = 0;
                /* ingredients는 2단계에서 채움 */
                recipeCount++;
            }
        }
    }
    fclose(frec);

    /* ── 2단계: recipe_ing.db 로드 ── */
    fing = fopen(RECIPE_ING_DB, "r");
    if (!fing) return;

    while (fgets(line, sizeof(line), fing)) {
        int  ingId, rid;
        char ingName[50];
        if (sscanf(line, "%d|%d|%49[^\n]", &ingId, &rid, ingName) != 3)
            continue;

        /* recipeId 일치하는 Recipe 구조체에 재료 추가 */
        for (i = 0; i < recipeCount; i++) {
            if (recipes[i].recipeId == rid &&
                recipes[i].ingCount < MAX_ING) {
                strcpy(recipes[i].ingredients[recipes[i].ingCount], ingName);
                recipes[i].ingCount++;
                break;
            }
        }
    }
    fclose(fing);
}

/* ════════════════════════════════════════════════════════════════════
 * 메인 메뉴 (3주차 + 4주차)
 * ════════════════════════════════════════════════════════════════════ */

void showMenu(void) {
    int imm = countImminent();

    printLine();
    printf("        FridgeChef - 냉장고 재고 & 레시피 관리\n");
    printLine();
    printf("\n");
    printf("  재료 %d개 | 레시피 %d개 로드 완료.\n", itemCount, recipeCount);
    printf("\n");
    printf("  +------------------------------------------------------+\n");
    printf("  |              [ 재료 관리 ]                           |\n");
    printf("  +------------------------------------------------------+\n");
    printf("  |  1.  재료 목록 보기                                  |\n");
    printf("  |  2.  재료 추가                                       |\n");
    printf("  |  3.  재료 삭제                                       |\n");
    printf("  |  4.  재료 수정                         [4주차]       |\n");
    printf("  |  5.  재료 검색                         [4주차]       |\n");
    printf("  |  6.  유통기한 순 정렬                  [4주차]       |\n");
    if (imm > 0)
        printf("  |  7.  유통기한 임박 확인     !! %2d개 임박!           |\n", imm);
    else
        printf("  |  7.  유통기한 임박 확인                              |\n");
    printf("  +------------------------------------------------------+\n");
    printf("  |              [ 레시피 관리 ]           [4주차]       |\n");
    printf("  +------------------------------------------------------+\n");
    printf("  |  8.  레시피 추천 (매칭률 계산)         [4주차]       |\n");
    printf("  |  9.  레시피 관리 (추가/목록/삭제)      [4주차]       |\n");
    printf("  +------------------------------------------------------+\n");
    printf("  |  0.  종료                                            |\n");
    printf("  +------------------------------------------------------+\n");
    printf("\n  선택 >> ");
}

/* ════════════════════════════════════════════════════════════════════
 * 재료 목록 (3주차 D-day 컬럼 유지)
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
 * [4주차] 재료 검색 (이름 부분 일치)
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
        /* 대소문자 무관 부분 일치 검색 */
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
 * [4주차] 유통기한 오름차순 정렬 (버블 정렬)
 *  - 날짜 문자열이 "YYYY-MM-DD" 형식이므로 strcmp로 사전순 비교 = 날짜순
 *  - "미정" 항목은 맨 뒤로 보냄
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
            if      ( a_mijeong && !b_mijeong) swap = 1;  /* 미정은 뒤로 */
            else if (!a_mijeong && !b_mijeong)
                swap = (strcmp(inventory[j].expiry, inventory[j+1].expiry) > 0);

            if (swap) {
                tmp             = inventory[j];
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
 * 재료 추가 (3주차 임박 경고 유지)
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

    /* 1. 이름 */
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

    /* 임박 경고 */
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
 * [4주차] 재료 수정 (수량 또는 유통기한 변경)
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
 * 재료 삭제 (3주차)
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
 * 유통기한 임박 확인 (3주차)
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
 * [4주차] 레시피 추가
 *  - recipeId : AUTO INCREMENT (nextRecipeId)
 *  - 레시피 이름 + 재료 목록 입력
 *  - 저장 후 recipes.db / recipe_ing.db 즉시 갱신
 * ════════════════════════════════════════════════════════════════════ */

void addRecipe(void) {
    Recipe nr;
    int    i;

    printf("\n");
    printLine();
    printf("  [레시피 추가]\n");
    printLine();

    if (recipeCount >= MAX_RECIPES) {
        printf("  레시피가 가득 찼습니다. (최대 %d개)\n", MAX_RECIPES);
        printf("  Enter 를 누르면 돌아갑니다...");
        flushInput(); getchar(); return;
    }

    flushInput();

    /* 레시피 이름 */
    printf("  레시피 이름 : ");
    fgets(nr.name, sizeof(nr.name), stdin);
    nr.name[strcspn(nr.name, "\n")] = '\0';
    if (strlen(nr.name) == 0) {
        printf("  이름을 입력하지 않았습니다. 취소합니다.\n");
        printf("  Enter 를 누르면 돌아갑니다...");
        getchar(); return;
    }

    /* 재료 수 */
    printf("  필요 재료 수 (최대 %d개) : ", MAX_ING);
    scanf("%d", &nr.ingCount);
    if (nr.ingCount <= 0 || nr.ingCount > MAX_ING) {
        printf("  잘못된 재료 수입니다. 취소합니다.\n");
        printf("  Enter 를 누르면 돌아갑니다...");
        flushInput(); getchar(); return;
    }
    flushInput();

    /* 재료 이름 입력 */
    for (i = 0; i < nr.ingCount; i++) {
        printf("  재료 %d : ", i + 1);
        fgets(nr.ingredients[i], sizeof(nr.ingredients[i]), stdin);
        nr.ingredients[i][strcspn(nr.ingredients[i], "\n")] = '\0';
        if (strlen(nr.ingredients[i]) == 0)
            strcpy(nr.ingredients[i], "미입력");
    }

    /* recipeId AUTO INCREMENT */
    nr.recipeId = nextRecipeId++;
    recipes[recipeCount++] = nr;
    saveRecipes();

    printf("\n");
    printLine();
    printf("  [%s] 레시피가 추가되었습니다! (ID: %d)\n", nr.name, nr.recipeId);
    printf("  필요 재료 (%d개): ", nr.ingCount);
    for (i = 0; i < nr.ingCount; i++)
        printf("%s%s", nr.ingredients[i], (i < nr.ingCount-1) ? ", " : "\n");
    printf("  현재 등록된 레시피: 총 %d개\n", recipeCount);
    printLine();

    printf("  Enter 를 누르면 돌아갑니다...");
    getchar();
}

/* ════════════════════════════════════════════════════════════════════
 * [4주차] 레시피 목록 출력
 * ════════════════════════════════════════════════════════════════════ */

void listRecipes(void) {
    int i, j;

    printf("\n");
    printLine();
    printf("  [레시피 목록] - 총 %d개\n", recipeCount);
    printLine();

    if (recipeCount == 0) {
        printf("  등록된 레시피가 없습니다.\n");
        printf("  메뉴 9번 → 1번으로 레시피를 추가해 보세요!\n");
    } else {
        for (i = 0; i < recipeCount; i++) {
            printf("  [%d] %s  (재료 %d개)\n",
                   recipes[i].recipeId,
                   recipes[i].name,
                   recipes[i].ingCount);
            printf("    재료: ");
            for (j = 0; j < recipes[i].ingCount; j++)
                printf("%s%s", recipes[i].ingredients[j],
                       (j < recipes[i].ingCount-1) ? ", " : "\n");
            if (i < recipeCount - 1)
                printf("  -----------------------------------------------\n");
        }
    }

    printf("\n  Enter 를 누르면 돌아갑니다...");
    flushInput();
    getchar();
}

/* ════════════════════════════════════════════════════════════════════
 * [4주차] 레시피 삭제
 * ════════════════════════════════════════════════════════════════════ */

void deleteRecipe(void) {
    int i, choice;

    printf("\n");
    printLine();
    printf("  [레시피 삭제]\n");
    printLine();

    if (recipeCount == 0) {
        printf("  삭제할 레시피가 없습니다.\n");
        printf("\n  Enter 를 누르면 돌아갑니다...");
        flushInput(); getchar(); return;
    }

    for (i = 0; i < recipeCount; i++)
        printf("  %3d. [ID:%d] %s\n", i+1, recipes[i].recipeId, recipes[i].name);
    printf("\n");

    printf("  삭제할 레시피 번호 (0: 취소) >> ");
    scanf("%d", &choice);

    if (choice <= 0 || choice > recipeCount) {
        printf("  취소하거나 잘못된 번호입니다.\n");
    } else {
        char delname[50];
        strcpy(delname, recipes[choice-1].name);
        for (i = choice - 1; i < recipeCount - 1; i++)
            recipes[i] = recipes[i + 1];
        recipeCount--;
        saveRecipes();
        printf("  [%s] 레시피가 삭제되었습니다. (남은 레시피: %d개)\n",
               delname, recipeCount);
    }

    printf("\n  Enter 를 누르면 돌아갑니다...");
    flushInput(); getchar();
}

/* ════════════════════════════════════════════════════════════════════
 * [4주차] 레시피 추천 — 보유 재료 매칭률 계산
 *
 * 알고리즘:
 *  1. 각 레시피마다 필요 재료 중 inventory에 있는 재료 수 / 전체 재료 수
 *     = 매칭률(%) 계산
 *  2. 매칭률 내림차순 정렬 후 상위 5개 출력
 *  3. 임박 재료 우선 추천 모드:
 *     임박 재료를 최대한 활용하는 레시피에 +20% 가중치 부여
 * ════════════════════════════════════════════════════════════════════ */

/* 재료명이 inventory에 있는지 확인 (부분 일치) */
static int hasIngredient(const char *ingName) {
    int i;
    char hay[50], needle[50];
    int k;
    strcpy(needle, ingName);
    for (k = 0; needle[k]; k++) needle[k] = (char)tolower((unsigned char)needle[k]);

    for (i = 0; i < itemCount; i++) {
        strcpy(hay, inventory[i].name);
        for (k = 0; hay[k]; k++) hay[k] = (char)tolower((unsigned char)hay[k]);
        if (strstr(hay, needle) || strstr(needle, hay))
            return 1;
    }
    return 0;
}

/* 임박 재료와 관련된 재료 수 반환 */
static int imminentMatchCount(const Recipe *r) {
    int i, j, cnt = 0;
    char hay[50], needle[50];
    int k;

    for (j = 0; j < r->ingCount; j++) {
        strcpy(needle, r->ingredients[j]);
        for (k = 0; needle[k]; k++) needle[k] = (char)tolower((unsigned char)needle[k]);

        for (i = 0; i < itemCount; i++) {
            if (daysUntilExpiry(inventory[i].expiry) > IMMINENT) continue;
            strcpy(hay, inventory[i].name);
            for (k = 0; hay[k]; k++) hay[k] = (char)tolower((unsigned char)hay[k]);
            if (strstr(hay, needle) || strstr(needle, hay)) { cnt++; break; }
        }
    }
    return cnt;
}

void showRecipes(void) {
    int  i, j;
    int  mode;          /* 1: 일반  2: 임박 우선 */

    /* 매칭 결과 */
    typedef struct { int idx; float score; int matched; } MatchResult;
    MatchResult results[MAX_RECIPES];
    int         rcount = 0;
    MatchResult tmp_r;

    printf("\n");
    printLine();
    printf("  [레시피 추천]\n");
    printLine();

    if (recipeCount == 0) {
        printf("  등록된 레시피가 없습니다.\n");
        printf("  메뉴 9번 → 1번으로 레시피를 추가해 보세요!\n");
        printf("\n  Enter 를 누르면 메인 메뉴로 돌아갑니다...");
        flushInput(); getchar(); return;
    }

    if (itemCount == 0) {
        printf("  재료가 없습니다. 먼저 재료를 추가해 주세요!\n");
        printf("\n  Enter 를 누르면 메인 메뉴로 돌아갑니다...");
        flushInput(); getchar(); return;
    }

    printf("  추천 모드를 선택하세요.\n");
    printf("    1. 일반 추천    (보유 재료 매칭률 기준)\n");
    printf("    2. 임박 재료 우선 추천  (임박 재료 활용 가중치 +20%%)\n");
    printf("  선택 >> ");
    scanf("%d", &mode);
    if (mode < 1 || mode > 2) mode = 1;

    /* ── 매칭률 계산 ── */
    for (i = 0; i < recipeCount; i++) {
        int matched = 0;
        float score;

        for (j = 0; j < recipes[i].ingCount; j++)
            if (hasIngredient(recipes[i].ingredients[j]))
                matched++;

        score = (recipes[i].ingCount > 0)
                ? (float)matched / recipes[i].ingCount * 100.0f
                : 0.0f;

        /* 임박 재료 가중치 */
        if (mode == 2) {
            int imm_cnt = imminentMatchCount(&recipes[i]);
            if (imm_cnt > 0)
                score += 20.0f * ((float)imm_cnt / recipes[i].ingCount);
            if (score > 100.0f) score = 100.0f;
        }

        results[rcount].idx     = i;
        results[rcount].score   = score;
        results[rcount].matched = matched;
        rcount++;
    }

    /* ── 버블 정렬 (내림차순) ── */
    for (i = 0; i < rcount - 1; i++) {
        for (j = 0; j < rcount - 1 - i; j++) {
            if (results[j].score < results[j+1].score) {
                tmp_r        = results[j];
                results[j]   = results[j+1];
                results[j+1] = tmp_r;
            }
        }
    }

    /* ── 결과 출력 (상위 5개) ── */
    printf("\n");
    printLine();
    printf("  [추천 레시피 TOP %d] %s\n",
           (rcount < 5 ? rcount : 5),
           mode == 2 ? "— 임박 재료 우선 모드" : "— 일반 모드");
    printLine();

    for (i = 0; i < rcount && i < 5; i++) {
        int ri = results[i].idx;
        printf("  %d위. [%s]  %.0f%% 매칭  (%d / %d 재료 보유)\n",
               i + 1,
               recipes[ri].name,
               results[i].score,
               results[i].matched,
               recipes[ri].ingCount);

        /* 보유/미보유 재료 상세 표시 */
        printf("       재료: ");
        for (j = 0; j < recipes[ri].ingCount; j++) {
            int have = hasIngredient(recipes[ri].ingredients[j]);
            printf("%s%s",
                   have ? "" : "[없음]",
                   recipes[ri].ingredients[j]);
            if (j < recipes[ri].ingCount - 1) printf(", ");
        }
        printf("\n");

        if (i < rcount - 1 && i < 4)
            printf("  -----------------------------------------------\n");
    }
    printLine();

    if (results[0].score == 0.0f)
        printf("  ※ 보유 재료로 만들 수 있는 레시피가 없습니다.\n"
               "    재료를 더 추가하거나 레시피를 등록해 보세요.\n");

    printf("\n  Enter 를 누르면 메인 메뉴로 돌아갑니다...");
    flushInput(); getchar();
}

/* ════════════════════════════════════════════════════════════════════
 * [4주차] 레시피 서브메뉴
 * ════════════════════════════════════════════════════════════════════ */

void recipeMenu(void) {
    int choice;

    while (1) {
        printf("\n");
        printLine();
        printf("  [레시피 관리]  (등록: %d개)\n", recipeCount);
        printLine();
        printf("  |  1.  레시피 추가                                     |\n");
        printf("  |  2.  레시피 목록 보기                                |\n");
        printf("  |  3.  레시피 삭제                                     |\n");
        printf("  |  0.  메인 메뉴로 돌아가기                            |\n");
        printLine();
        printf("  선택 >> ");
        scanf("%d", &choice);

        switch (choice) {
            case 1: addRecipe();    break;
            case 2: listRecipes();  break;
            case 3: deleteRecipe(); break;
            case 0: return;
            default:
                printf("  잘못된 입력입니다.\n");
                printf("  Enter 를 누르면 계속...");
                flushInput(); getchar();
                break;
        }
    }
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

    loadItems();     /* 재료 DB 복원 */
    loadRecipes();   /* [4주차] 레시피 DB 복원 */

    while (1) {
#ifdef _WIN32
        system("cls");
#else
        system("clear");
#endif
        showMenu();
        scanf("%d", &choice);

        switch (choice) {
            case 1: listItems();        break;
            case 2: addItem();          break;
            case 3: deleteItem();       break;
            case 4: editItem();         break;  /* [4주차] */
            case 5: searchItem();       break;  /* [4주차] */
            case 6: sortItemsByExpiry();break;  /* [4주차] */
            case 7: checkAlert();       break;
            case 8: showRecipes();      break;  /* [4주차] */
            case 9: recipeMenu();       break;  /* [4주차] */
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
