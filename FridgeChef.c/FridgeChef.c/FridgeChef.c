/*
 * FridgeChef - 냉장고 재고 & 레시피 관리
 * C언어 콘솔 버전
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_ITEMS    100
#define MAX_RECIPES  10
#define MAX_ING      5
#define IMMINENT     3
#define FILENAME     "inventory.txt"

/* ── 구조체 정의 ──────────────────────────────────── */

typedef struct {
    char name[50];
    char category[20];
    char location[10];
    int  quantity;
    char expiry[11];   /* YYYY-MM-DD */
} Item;

typedef struct {
    char name[50];
    char ingredients[MAX_ING][50];
    int  ingCount;
} Recipe;

/* ── 전역 변수 ────────────────────────────────────── */

Item   inventory[MAX_ITEMS];
int    itemCount = 0;

Recipe recipes[MAX_RECIPES];
int    recipeCount = 0;

/* ── D-Day 계산 ───────────────────────────────────── */

int calcDDay(const char *expiry) {
    struct tm exp = {0};
    int y, m, d;
    time_t today, expTime;

    if (sscanf(expiry, "%d-%d-%d", &y, &m, &d) != 3)
        return 9999;

    exp.tm_year = y - 1900;
    exp.tm_mon  = m - 1;
    exp.tm_mday = d;
    exp.tm_hour = 12;

    today   = time(NULL);
    expTime = mktime(&exp);

    return (int)((expTime - today) / 86400);
}

/* ── 파일 저장 / 불러오기 ─────────────────────────── */

void saveToFile(void) {
    FILE *fp = fopen(FILENAME, "w");
    int i;
    if (fp == NULL) {
        printf("  파일 저장 실패!\n");
        return;
    }
    for (i = 0; i < itemCount; i++) {
        fprintf(fp, "%s %s %s %d %s\n",
                inventory[i].name,
                inventory[i].category,
                inventory[i].location,
                inventory[i].quantity,
                inventory[i].expiry);
    }
    fclose(fp);
}

void loadFromFile(void) {
    FILE *fp = fopen(FILENAME, "r");
    if (fp == NULL) return;
    itemCount = 0;
    while (itemCount < MAX_ITEMS &&
           fscanf(fp, "%s %s %s %d %s",
                  inventory[itemCount].name,
                  inventory[itemCount].category,
                  inventory[itemCount].location,
                  &inventory[itemCount].quantity,
                  inventory[itemCount].expiry) == 5) {
        itemCount++;
    }
    fclose(fp);
}

/* ── 레시피 초기 데이터 ───────────────────────────── */

void initRecipes(void) {
    /* 1. 계란볶음밥 */
    strcpy(recipes[0].name, "계란볶음밥");
    strcpy(recipes[0].ingredients[0], "계란");
    strcpy(recipes[0].ingredients[1], "밥");
    strcpy(recipes[0].ingredients[2], "대파");
    strcpy(recipes[0].ingredients[3], "간장");
    recipes[0].ingCount = 4;

    /* 2. 두부된장찌개 */
    strcpy(recipes[1].name, "두부된장찌개");
    strcpy(recipes[1].ingredients[0], "두부");
    strcpy(recipes[1].ingredients[1], "된장");
    strcpy(recipes[1].ingredients[2], "대파");
    recipes[1].ingCount = 3;

    /* 3. 시금치나물 */
    strcpy(recipes[2].name, "시금치나물");
    strcpy(recipes[2].ingredients[0], "시금치");
    strcpy(recipes[2].ingredients[1], "참기름");
    recipes[2].ingCount = 2;

    /* 4. 삼겹살구이 */
    strcpy(recipes[3].name, "삼겹살구이");
    strcpy(recipes[3].ingredients[0], "삼겹살");
    strcpy(recipes[3].ingredients[1], "상추");
    strcpy(recipes[3].ingredients[2], "쌈장");
    strcpy(recipes[3].ingredients[3], "마늘");
    strcpy(recipes[3].ingredients[4], "고추");
    recipes[3].ingCount = 5;

    recipeCount = 4;
}

/* ── 보유 재료 확인 ───────────────────────────────── */

int hasIngredient(const char *name) {
    int i;
    for (i = 0; i < itemCount; i++) {
        if (strcmp(inventory[i].name, name) == 0 &&
            inventory[i].quantity > 0)
            return 1;
    }
    return 0;
}

/* 해당 재료의 D-Day 반환 */
int getIngredientDDay(const char *name) {
    int i;
    for (i = 0; i < itemCount; i++) {
        if (strcmp(inventory[i].name, name) == 0)
            return calcDDay(inventory[i].expiry);
    }
    return 9999;
}

/* ── 임박 재료 수 카운트 ──────────────────────────── */

int countImminent(void) {
    int count = 0, i;
    for (i = 0; i < itemCount; i++) {
        int d = calcDDay(inventory[i].expiry);
        if (d <= IMMINENT) count++;
    }
    return count;
}

/* ── 함수 전방 선언 ───────────────────────────────── */

void showRecipes(void);

/* ── 구분선 ───────────────────────────────────────── */

void printLine(void) {
    printf("================================================================\n");
}

/* ── 1. 재료 목록 보기 ────────────────────────────── */

void listItems(void) {
    int i, d;

    printLine();
    printf("                 재료 목록 (총 %d개)                        \n", itemCount);
    printLine();
    printf("\n");

    printf("  번호  재료명       카테고리   보관   수량   D-Day   상태\n");
    printf("  ──────────────────────────────────────────────────────────\n");

    for (i = 0; i < itemCount; i++) {
        d = calcDDay(inventory[i].expiry);
        printf("  [%d]   %-12s %-10s %-6s %3d   ",
               i + 1,
               inventory[i].name,
               inventory[i].category,
               inventory[i].location,
               inventory[i].quantity);

        if (d < 0)
            printf("D+%d    X 만료\n", -d);
        else if (d <= IMMINENT)
            printf("D-%d    △ 임박\n", d);
        else
            printf("D-%d   ✔ 여유\n", d);
    }

    printf("  ──────────────────────────────────────────────────────────\n");
    printf("\n  초록: 여유 (D-4이상)    주황: 임박 (D-3이하)    빨강: 만료\n");
    printf("\n  Enter 를 누르면 메인 메뉴로 돌아갑니다...");
    getchar(); getchar();
}

/* ── 2. 재료 추가 ─────────────────────────────────── */

void addItem(void) {
    Item newItem;
    int d;
    char confirm;

    if (itemCount >= MAX_ITEMS) {
        printf("  재고가 가득 찼습니다! (최대 %d개)\n", MAX_ITEMS);
        return;
    }

    printLine();
    printf("                      재료 추가                            \n");
    printLine();
    printf("\n");
    printf("  아래 항목을 순서대로 입력하세요.\n");
    printf("  (취소하려면 빈 칸으로 Enter)\n\n");
    printf("  ──────────────────────────────────────────────────────────\n");

    printf("  재료명                        >> ");
    scanf("%s", newItem.name);

    printf("  카테고리 (야채/육류/유제품/기타) >> ");
    scanf("%s", newItem.category);

    printf("  보관 위치 (냉장/냉동/실온)     >> ");
    scanf("%s", newItem.location);

    printf("  수량                          >> ");
    scanf("%d", &newItem.quantity);

    printf("  유통기한 (YYYY-MM-DD)         >> ");
    scanf("%s", newItem.expiry);

    d = calcDDay(newItem.expiry);

    printf("\n  ──────────────────────────────────────────────────────────\n");
    printf("  입력 내용 확인:\n");
    printf("    이름     : %s\n", newItem.name);
    printf("    카테고리 : %s\n", newItem.category);
    printf("    보관     : %s\n", newItem.location);
    printf("    수량     : %d\n", newItem.quantity);
    printf("    유통기한 : %s  (D-%d)\n", newItem.expiry, d);

    printf("\n  저장하시겠습니까? (y/n) >> ");
    scanf(" %c", &confirm);

    if (confirm == 'y' || confirm == 'Y') {
        inventory[itemCount] = newItem;
        itemCount++;
        saveToFile();
        printf("\n  ✔ %s가 추가되었습니다! (총 %d개)\n", newItem.name, itemCount);
    } else {
        printf("\n  취소되었습니다.\n");
    }

    printf("  Enter 를 누르면 메인 메뉴로 돌아갑니다...");
    getchar(); getchar();
}

/* ── 3. 재료 삭제 ─────────────────────────────────── */

void deleteItem(void) {
    int i, d, choice;
    char confirm;

    printLine();
    printf("                      재료 삭제                            \n");
    printLine();
    printf("\n");

    printf("  번호  재료명       카테고리   보관   수량   상태\n");
    printf("  ──────────────────────────────────────────────────────────\n");

    for (i = 0; i < itemCount; i++) {
        d = calcDDay(inventory[i].expiry);
        printf("  [%d]   %-12s %-10s %-6s %3d   ",
               i + 1,
               inventory[i].name,
               inventory[i].category,
               inventory[i].location,
               inventory[i].quantity);

        if (d < 0)
            printf("X 만료\n");
        else if (d <= IMMINENT)
            printf("△ 임박\n");
        else
            printf("✔ 여유\n");
    }

    printf("  ──────────────────────────────────────────────────────────\n");
    printf("\n  삭제할 번호를 입력하세요 (0: 취소) >> ");
    scanf("%d", &choice);

    if (choice < 1 || choice > itemCount) {
        printf("  취소되었습니다.\n");
        printf("  Enter 를 누르면 메인 메뉴로 돌아갑니다...");
        getchar(); getchar();
        return;
    }

    printf("\n  [%s] 를 삭제하시겠습니까? (y/n) >> ", inventory[choice - 1].name);
    scanf(" %c", &confirm);

    if (confirm == 'y' || confirm == 'Y') {
        char deletedName[50];
        strcpy(deletedName, inventory[choice - 1].name);

        for (i = choice - 1; i < itemCount - 1; i++) {
            inventory[i] = inventory[i + 1];
        }
        itemCount--;
        saveToFile();
        printf("\n  ✔ %s가 삭제되었습니다. (총 %d개)\n", deletedName, itemCount);
    } else {
        printf("  취소되었습니다.\n");
    }

    printf("  Enter 를 누르면 메인 메뉴로 돌아갑니다...");
    getchar(); getchar();
}

/* ── 4. 유통기한 임박 확인 ────────────────────────── */

void checkAlert(void) {
    int i, d, alertCount = 0;
    char goRecipe;

    printLine();
    printf("              유통기한 임박 재료  (D-%d 이하)                \n", IMMINENT);
    printLine();
    printf("\n");

    printf("  ┌────────────┬────────────────┬───────┬────────┐\n");
    printf("  │  재료명    │  유통기한      │ D-Day │  상태  │\n");
    printf("  ├────────────┼────────────────┼───────┼────────┤\n");

    for (i = 0; i < itemCount; i++) {
        d = calcDDay(inventory[i].expiry);
        if (d <= IMMINENT) {
            alertCount++;
            if (d < 0)
                printf("  │  %-10s│  %-14s│  D+%-3d│ X 만료 │\n",
                       inventory[i].name, inventory[i].expiry, -d);
            else
                printf("  │  %-10s│  %-14s│  D-%-3d│ △ 임박 │\n",
                       inventory[i].name, inventory[i].expiry, d);
        }
    }

    printf("  └────────────┴────────────────┴───────┴────────┘\n");
    printf("\n  총 %d개 재료가 임박하거나 만료되었습니다.\n", alertCount);

    if (alertCount == 0) {
        printf("\n  임박한 재료가 없습니다!\n");
    } else {
        printf("\n  이 재료들로 만들 수 있는 레시피를 보시겠습니까?\n");
        printf("  (y: 레시피 추천 이동  /  n: 메인 메뉴) >> ");
        scanf(" %c", &goRecipe);
        if (goRecipe == 'y' || goRecipe == 'Y') {
            showRecipes();
            return;
        }
    }

    printf("  Enter 를 누르면 메인 메뉴로 돌아갑니다...");
    getchar(); getchar();
}

/* ── 레시피 매칭 정보 구조체 ──────────────────────── */

typedef struct {
    int   recipeIdx;
    int   matchCount;
    int   imminentCount;
    int   missingCount;
    int   matchPercent;
} RecipeMatch;

/* ── 5. 레시피 추천 ───────────────────────────────── */

void showRecipes(void) {
    RecipeMatch matches[MAX_RECIPES];
    int i, j, d, choice;


    printLine();
    printf("               레시피 추천  (매칭률 높은 순)                \n");
    printLine();
    printf("\n");

    /* 보유 재료 표시 */
    printf("  현재 보유 재료: ");
    for (i = 0; i < itemCount; i++) {
        printf("%s", inventory[i].name);
        if (i < itemCount - 1) printf(", ");
    }
    printf("\n\n");

    /* 매칭률 계산 */
    for (i = 0; i < recipeCount; i++) {
        int have = 0, imm = 0, miss = 0;
        for (j = 0; j < recipes[i].ingCount; j++) {
            if (hasIngredient(recipes[i].ingredients[j])) {
                have++;
                d = getIngredientDDay(recipes[i].ingredients[j]);
                if (d <= IMMINENT) imm++;
            } else {
                miss++;
            }
        }
        matches[i].recipeIdx     = i;
        matches[i].matchCount    = have;
        matches[i].imminentCount = imm;
        matches[i].missingCount  = miss;
        matches[i].matchPercent  = (recipes[i].ingCount > 0)
            ? (have * 100 / recipes[i].ingCount) : 0;
    }

    /* 매칭률 순 정렬 (내림차순) */
    for (i = 0; i < recipeCount - 1; i++) {
        for (j = i + 1; j < recipeCount; j++) {
            if (matches[j].matchPercent > matches[i].matchPercent) {
                RecipeMatch tmp = matches[i];
                matches[i] = matches[j];
                matches[j] = tmp;
            }
        }
    }

    /* 테이블 출력 */
    printf("  ┌──────┬───────────────┬────────┬────────┬────────┐\n");
    printf("  │ 순위 │  레시피명     │ 매칭률 │ 임박   │ 부족   │\n");
    printf("  ├──────┼───────────────┼────────┼────────┼────────┤\n");

    for (i = 0; i < recipeCount; i++) {
        int idx = matches[i].recipeIdx;
        printf("  │  %d   │  %-12s │  %3d%%  │  %d개   │  %d개   │\n",
               i + 1,
               recipes[idx].name,
               matches[i].matchPercent,
               matches[i].imminentCount,
               matches[i].missingCount);
    }

    printf("  └──────┴───────────────┴────────┴────────┴────────┘\n");

    /* 상세 보기 */
    while (1) {
        printf("\n  상세 보기 (번호 입력  /  0: 메인 메뉴) >> ");
        scanf("%d", &choice);

        if (choice == 0) break;

        if (choice < 1 || choice > recipeCount) {
            printf("  잘못된 번호입니다.\n");
            continue;
        }

        i = choice - 1;
        int idx = matches[i].recipeIdx;

        printf("\n  ┌─────────────────────────────────────────────────┐\n");
        printf("  │  [ %d순위 ]  %s  (매칭률 %d%%)            │\n",
               choice, recipes[idx].name, matches[i].matchPercent);
        printf("  ├─────────────────────────────────────────────────┤\n");

        for (j = 0; j < recipes[idx].ingCount; j++) {
            if (hasIngredient(recipes[idx].ingredients[j])) {
                d = getIngredientDDay(recipes[idx].ingredients[j]);
                if (d <= IMMINENT && d >= 0)
                    printf("  │  △ %-10s 보유중  (임박 D-%d)                 │\n",
                           recipes[idx].ingredients[j], d);
                else
                    printf("  │  ✔ %-10s 보유중                             │\n",
                           recipes[idx].ingredients[j]);
            } else {
                printf("  │  ✘ %-10s 없음 - 추가 구매 필요              │\n",
                       recipes[idx].ingredients[j]);
            }
        }

        printf("  └─────────────────────────────────────────────────┘\n");
        printf("\n  다른 번호 입력  /  0: 메인 메뉴 >> ");
        scanf("%d", &choice);
        if (choice == 0) break;

        /* 재선택 시 다시 루프 */
        if (choice >= 1 && choice <= recipeCount) {
            i = choice - 1;
            idx = matches[i].recipeIdx;

            printf("\n  ┌─────────────────────────────────────────────────┐\n");
            printf("  │  [ %d순위 ]  %s  (매칭률 %d%%)            │\n",
                   choice, recipes[idx].name, matches[i].matchPercent);
            printf("  ├─────────────────────────────────────────────────┤\n");

            for (j = 0; j < recipes[idx].ingCount; j++) {
                if (hasIngredient(recipes[idx].ingredients[j])) {
                    d = getIngredientDDay(recipes[idx].ingredients[j]);
                    if (d <= IMMINENT && d >= 0)
                        printf("  │  △ %-10s 보유중  (임박 D-%d)                 │\n",
                               recipes[idx].ingredients[j], d);
                    else
                        printf("  │  ✔ %-10s 보유중                             │\n",
                               recipes[idx].ingredients[j]);
                } else {
                    printf("  │  ✘ %-10s 없음 - 추가 구매 필요              │\n",
                           recipes[idx].ingredients[j]);
                }
            }
            printf("  └─────────────────────────────────────────────────┘\n");
        }
        break;
    }
}

/* ── 기본 샘플 데이터 ─────────────────────────────── */

void loadSampleData(void) {
    /* 파일이 없거나 비어있으면 샘플 데이터 로드 */
    if (itemCount > 0) return;

    strcpy(inventory[0].name, "계란");
    strcpy(inventory[0].category, "유제품");
    strcpy(inventory[0].location, "냉장");
    inventory[0].quantity = 6;
    strcpy(inventory[0].expiry, "2026-05-10");

    strcpy(inventory[1].name, "두부");
    strcpy(inventory[1].category, "야채");
    strcpy(inventory[1].location, "냉장");
    inventory[1].quantity = 1;
    strcpy(inventory[1].expiry, "2026-04-29");

    strcpy(inventory[2].name, "삼겹살");
    strcpy(inventory[2].category, "육류");
    strcpy(inventory[2].location, "냉동");
    inventory[2].quantity = 2;
    strcpy(inventory[2].expiry, "2026-05-27");

    strcpy(inventory[3].name, "시금치");
    strcpy(inventory[3].category, "야채");
    strcpy(inventory[3].location, "냉장");
    inventory[3].quantity = 1;
    strcpy(inventory[3].expiry, "2026-04-26");

    strcpy(inventory[4].name, "우유");
    strcpy(inventory[4].category, "유제품");
    strcpy(inventory[4].location, "냉장");
    inventory[4].quantity = 1;
    strcpy(inventory[4].expiry, "2026-04-28");

    itemCount = 5;
    saveToFile();
}

/* ── 메인 메뉴 ────────────────────────────────────── */

void showMenu(void) {
    int imm = countImminent();

    printLine();
    printf("        FridgeChef - 냉장고 재고 & 레시피 관리          \n");
    printLine();
    printf("\n");
    printf("  데이터 불러오는 중...\n");
    printf("  재료 %d개 로드 완료.\n", itemCount);
    printf("\n");
    printf("  ┌──────────────────────────────────────────────────────┐\n");
    printf("  │                    메인 메뉴                          │\n");
    printf("  ├──────────────────────────────────────────────────────┤\n");
    printf("  │  1.  재료 목록 보기                                   │\n");
    printf("  │  2.  재료 추가                                        │\n");
    printf("  │  3.  재료 삭제                                        │\n");

    if (imm > 0)
        printf("  │  4.  유통기한 임박 확인          △  %d개 임박!        │\n", imm);
    else
        printf("  │  4.  유통기한 임박 확인                               │\n");

    printf("  │  5.  레시피 추천                                      │\n");
    printf("  │  0.  종료                                             │\n");
    printf("  └──────────────────────────────────────────────────────┘\n");
    printf("\n  선택 >> ");
}

/* ── 메인 함수 ────────────────────────────────────── */

int main(void) {
    int choice;
    char confirm;

    /* 초기화 */
    initRecipes();
    loadFromFile();
    loadSampleData();

    while (1) {
        /* 화면 지우기 (Windows: cls / 기타: clear) */
#ifdef _WIN32
        system("cls");
#else
        system("clear");
#endif
        showMenu();
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                listItems();
                break;
            case 2:
                addItem();
                break;
            case 3:
                deleteItem();
                break;
            case 4:
                checkAlert();
                break;
            case 5:
                showRecipes();
                break;
            case 0:
                printf("\n  정말 종료하시겠습니까? (y/n) >> ");
                scanf(" %c", &confirm);
                if (confirm == 'y' || confirm == 'Y') {
                    saveToFile();
                    printf("\n  데이터 저장 중...\n");
                    printf("  ✔ %s 저장 완료.\n\n", FILENAME);
                    printLine();
                    printf("    FridgeChef 를 종료합니다. 이용해주셔서 감사합니다!\n");
                    printLine();
                    return 0;
                }
                break;
            default:
                printf("  잘못된 입력입니다. 다시 선택하세요.\n");
                printf("  Enter 를 누르면 메인 메뉴로 돌아갑니다...");
                getchar(); getchar();
                break;
        }
    }

    return 0;
}
