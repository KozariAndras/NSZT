#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <time.h>
#define clear() printf("\033[H\033[J")

const char fileName[] = "szolo.dat";
const int stringSize = 256;

typedef struct Szallitmany
{
    int id;
    char videk[256];
    char termelo[256];
    int mennyiseg;
    char fajta[256];

} Szallitmany;

typedef struct Feldolgozas
{
    char fajta[256];
    int szoloMennyiseg;
    float borMennyiseg;

} Feldolgozas;

Szallitmany *szallitmanyDB;
Feldolgozas *feldolgozasDB;
int numOfSzallitmany;
int numOfFeldolgozas;

// Szolo.dat inicializálás alap értékekkel
void inicialize()
{

    if (access(fileName, F_OK) != 0)
    {
        // file nem létezik
        const char data1[] = "1,Balatoni borvidék,Gipsz Jakab,10,Olaszrizling\n";
        const char data2[] = "2,Tokaji borvidék,Teleki Péter,30,Muskotályos\n";

        FILE *fptr;

        fptr = fopen(fileName, "w+");
        if (fptr == NULL)
        {
            printf("Hiba a file(%s) írása közben", fileName);
            exit(1);
        }

        fseek(fptr, 0, SEEK_SET);
        fprintf(fptr, data1);
        fprintf(fptr, data2);
        fclose(fptr);
    }
}

void printSep()
{
    printf("---------------------\n");
}

// Dinamikusan bővíti az adatbázist
// Pataméter: Adatok száma
void resizeDatabase(int newSize)
{
    if (newSize == 0)
    {
        return;
    }

    szallitmanyDB = (Szallitmany *)realloc(szallitmanyDB, (newSize * sizeof(Szallitmany)));
    if (szallitmanyDB == NULL)
    {
        printf("Hiba az adatbázis bővítése közben!\n");
        exit(1);
    }
    return;
}

// Adatbázis feltöltése, visszatérési érték az adatok száma
int loadData()
{
    FILE *fptr;
    fptr = fopen(fileName, "r");
    if (fptr == NULL)
    {
        printf("Hiba a file(%s) olvasása közben", fileName);
    }
    fseek(fptr, 0, SEEK_SET);

    char line[sizeof(Szallitmany)];
    int lineNum = 0;
    while (fgets(line, sizeof(Szallitmany), fptr))
    {
        line[strlen(line) - 1] = '\0';

        char *token = strtok(line, ",");
        szallitmanyDB[lineNum].id = atoi(token);

        token = strtok(NULL, ",");
        strcpy(szallitmanyDB[lineNum].videk, token);

        token = strtok(NULL, ",");
        strcpy(szallitmanyDB[lineNum].termelo, token);

        token = strtok(NULL, ",");
        szallitmanyDB[lineNum].mennyiseg = atoi(token);

        token = strtok(NULL, ",");
        strcpy(szallitmanyDB[lineNum].fajta, token);
        // database[lineNum].id = lineNum + 1;

        lineNum++;
    }

    fclose(fptr);
    resizeDatabase(lineNum);
    return lineNum;
}

bool notWhiteSpaceOrNUll(char *word)
{

    int length = strlen(word);
    if (word == NULL)
    {
        return false;
    }

    if (length == 0)
    {
        return false;
    }

    int i = 0;
    while (i < length)
    {
        if ((int)word[i] != ' ')
        {
            break;
        }
        i++;
    }
    return i < length;
}

char *readVidek()
{
    printf("A borvidék neve: ");
    char *videk = calloc(stringSize, sizeof(char));
    do
    {
        fgets(videk, stringSize, stdin);
        videk[strlen(videk) - 1] = '\0';
    } while (!notWhiteSpaceOrNUll(videk));
    return videk;
}

char *readTermelo()
{
    printf("A termelő neve: ");
    char *termelo = calloc(stringSize, sizeof(char));
    do
    {
        fgets(termelo, stringSize, stdin);
        termelo[strlen(termelo) - 1] = '\0';
    } while (!notWhiteSpaceOrNUll(termelo));
    return termelo;
}

int readMennyiseg()
{
    int success = 0;
    int mennyiseg;

    while (success == 0)
    {
        printf("Mennyiség: ");
        success = scanf("%i", &mennyiseg);
        if (success == 0)
        {
            printf("Hibás mennyiség!\n");
        }
        while (fgetc(stdin) != '\n')
            ;
    }
    return mennyiseg;
}

char *readFajta()
{
    printf("Szőlő fajta: ");
    char *fajta = calloc(stringSize, sizeof(char));
    do
    {
        fgets(fajta, stringSize, stdin);
        fajta[strlen(fajta) - 1] = '\0';
    } while (!notWhiteSpaceOrNUll(fajta));
    return fajta;
}

// Paraméter: új adat
void saveNewData(Szallitmany data)
{

    FILE *fptr;
    fptr = fopen(fileName, "a");
    if (fptr == NULL)
    {
        printf("Hiba a file(%s) megnyitása közben\n", fileName);
    }
    fprintf(fptr, "%i,%s,%s,%i,%s\n", data.id, data.videk, data.termelo, data.mennyiseg, data.fajta);
    fclose(fptr);
}

void saveAllData()
{
    FILE *fptr;
    fptr = fopen(fileName, "w");
    if (fptr == NULL)
    {
        printf("Hiba a file(%s) megnyitása közben\n", fileName);
    }
    fseek(fptr, 0, SEEK_SET);

    for (int i = 0; i < numOfSzallitmany; i++)
    {
        if (szallitmanyDB[i].id > 0)
        {
            fprintf(fptr, "%i,%s,%s,%i,%s\n", szallitmanyDB[i].id, szallitmanyDB[i].videk, szallitmanyDB[i].termelo, szallitmanyDB[i].mennyiseg, szallitmanyDB[i].fajta);
        }
    }
    fclose(fptr);
}

// Egy szállítmány kiírása a konzolra
void printData(Szallitmany data)
{
    printf("\n");
    printf("  Vidék: \t %s \n", data.videk);
    printf("  Termelő neve:  %s \n", data.termelo);
    printf("  Mennyiség: \t %i kg\n", data.mennyiseg);
    printf("  Szőlő fajta: \t %s \n", data.fajta);
    printf("  Id: \t\t %i \n", data.id);
}

void printMainMenu()
{
    printSep();
    printf("NSZT\n");
    printSep();
    printf("(1) Összes adat listázása \n");
    printf("(2) Vidék szerinti listázás \n");
    printf("(3) Új szállítmány fogadása \n");
    printf("(4) Szállítmány módosítása \n");
    printf("(5) Szállítmány törlése \n");
    printf("(6) Szőlő feldolgozás \n");
    printf("(7) Kilépés\n");
}

void printAllData()
{
    clear();
    printSep();
    printf("Összes nyilvántartott adat: \n");
    for (int i = 0; i < numOfSzallitmany; i++)
    {
        printData(szallitmanyDB[i]);
    }
    printSep();
}

void printDataByOrigin()
{
    clear();
    printSep();
    char *userInput = readVidek();

    clear();
    printf("Összes nyilvántartott adat a(z) %s-ről: \n", userInput);
    for (int i = 0; i < numOfSzallitmany; i++)
    {
        if (strcmp(userInput, szallitmanyDB[i].videk) == 0)
        {
            printData(szallitmanyDB[i]);
        }
    }
}

// Paraméter: üzenet a felhasználónak
bool readUserConfirm(char *msg)
{
    printSep();
    printf("%s (y/n): ", msg);

    char toSave;
    while (true)
    {
        toSave = fgetc(stdin);
        if (toSave == 'y')
        {
            return true;
            clear();
        }
        else if (toSave == 'n')
        {
            return false;
            clear();
        }
        while (fgetc(stdin) != '\n')
            ;
    }
}

// Egy egy szállítmány adatainak beolvasása a konzolról és mentése file-ba
void reciveNewShipment()
{
    clear();
    printSep();

    char *videk = readVidek();
    char *termelo = readTermelo();
    int mennyiseg = readMennyiseg();
    char *fajta = readFajta();
    int maxId = 0;

    for (int i = 0; i < numOfSzallitmany; i++)
    {
        if (szallitmanyDB[i].id > maxId)
        {
            maxId = szallitmanyDB[i].id;
        }
    }
    maxId++;

    Szallitmany ujSzallitmany;

    strcpy(ujSzallitmany.videk, videk);
    strcpy(ujSzallitmany.termelo, termelo);
    ujSzallitmany.mennyiseg = mennyiseg;
    strcpy(ujSzallitmany.fajta, fajta);
    ujSzallitmany.id = maxId;

    free(videk);
    free(termelo);
    free(fajta);

    clear();
    printSep();
    printf("Megadott adatok: \n");
    printData(ujSzallitmany);

    if (readUserConfirm("Biztosan szeretné menteni ezeket az adatokat?"))
    {
        resizeDatabase(numOfSzallitmany + 1);
        saveNewData(ujSzallitmany);
        numOfSzallitmany = loadData();
        clear();
        printf("Sikeres mentés!\n");
    }
    else
    {
        clear();
        printf("Nem történt mentés!\n");
    }
}

int readInt()
{
    int userInput;
    int success = 0;
    while (success == 0)
    {
        printf("Válasszon: ");
        success = scanf("%i", &userInput);
        if (success == 0)
        {
            printf("Hibás bemenet!\n");
        }
        while (fgetc(stdin) != '\n')
            ;
    }
    return userInput;
}

void printModifyMenu(Szallitmany data)
{
    printf("(1) Vidék: %s\n", data.videk);
    printf("(2) Termelő: %s\n", data.termelo);
    printf("(3) Mennyiség: %i\n", data.mennyiseg);
    printf("(4) Fajta: %s\n", data.fajta);
    printf("(5) Mégse \n");
}

void modifyShippment(Szallitmany selectedShippment)
{
    while (true)
    {
        clear();
        printModifyMenu(selectedShippment);
        bool validChoice = true;

        int menuNum = readInt();
        if (menuNum == 5)
        {
            clear();
            return;
        }
        else if (menuNum == 1)
        {
            char *videk = readVidek();
            strcpy(selectedShippment.videk, videk);
            free(videk);
        }
        else if (menuNum == 2)
        {
            char *termelo = readTermelo();
            strcpy(selectedShippment.termelo, termelo);
            free(termelo);
        }
        else if (menuNum == 3)
        {
            selectedShippment.mennyiseg = readMennyiseg();
        }
        else if (menuNum == 4)
        {
            char *fajta = readFajta();
            strcpy(selectedShippment.fajta, fajta);
            free(fajta);
        }
        else
            validChoice = false;

        if (validChoice && readUserConfirm("Biztosan szeretné menteni a modósításokat?"))
        {
            for (int i = 0; i < numOfSzallitmany; i++)
            {
                if (szallitmanyDB[i].id == selectedShippment.id)
                {
                    szallitmanyDB[i] = selectedShippment;
                    saveAllData();
                    clear();
                    printf("Sikeres módosítás!\n");
                    return;
                }
            }
        }
        else if (validChoice)
        {
            clear();
            printf("Nem történt mentés!\n");
            return;
        }
    }
}

void modifyShippmentMenuController()
{
    while (true)
    {
        printAllData();
        printf("Vissza a menübe: \"0\"\n");
        printf("(Választás id beírásával)\n");

        int menuNum = readInt();
        if (menuNum == 0)
        {
            clear();
            return;
        }
        for (int i = 0; i < numOfSzallitmany; i++)
        {
            if (szallitmanyDB[i].id == menuNum)
            {
                Szallitmany selectedShippment = szallitmanyDB[i];
                modifyShippment(selectedShippment);
                return;
            }
        }
    }
}

void deleteShippment(int id)
{
    for (int i = 0; i < numOfSzallitmany; i++)
    {
        if (szallitmanyDB[i].id == id)
        {
            szallitmanyDB[i].id = -1;
        }
    }
    saveAllData();
    resizeDatabase(numOfSzallitmany - 1);
    numOfSzallitmany = loadData();

    return;
}

void deleteShippmentMenuController()
{
    while (true)
    {
        clear();
        printAllData();
        printf("(0) Vissza a menübe \n");
        printf("Írd be a törölni kívánt szállítmány id-t: \n");

        int menuNum = readInt();
        if (menuNum == 0)
        {
            return;
        }
        for (int i = 0; i < numOfSzallitmany; i++)
        {
            if (szallitmanyDB[i].id == menuNum)
            {
                if (readUserConfirm("Biztosan törölni szeretné ezt a rekordot?"))
                {
                    deleteShippment(menuNum);
                    clear();
                    printf("Sikeres törlés!\n");
                    return;
                }
                else
                {
                    clear();
                    printf("Nem történt törlés!\n");
                    return;
                }
            }
        }
    }
}

// 2. rész/SZFÜ(szőlő feldolgozó üzem)-------------------------------------

void printProduct(Feldolgozas item)
{
    printf("\n\tA szőlő fajtája: %s\n", item.fajta);
    printf("\tA szőlő mennyisége: %i kg\n", item.szoloMennyiseg);
    printf("\tA szőlőből készült bor mennyisége: %.2f l\n\n", item.borMennyiseg);
}

void printAllProducts(char *msg)
{
    clear();

    int i = 0;
    printSep();
    printf("%s", msg);
    while (i < numOfFeldolgozas)
    {
        printProduct(feldolgozasDB[i]);
        i++;
    }
}

bool selectGrapesForProcessing()
{
    if (numOfSzallitmany == 0)
    {
        printf("Az adatbázis nem tartalmaz adatokat, ez a művelet nem használható!\n");
        return false;
    }

    clear();
    printSep();
    printf("Adja meg a minimum mennyiséget a feldolgozáshoz!\n");

    int minAmount = readInt();

    int i = 0;
    int tempDBLen = 0;
    Feldolgozas *tempDB = calloc(numOfSzallitmany, sizeof(Feldolgozas));

    while (i < numOfSzallitmany)
    {
        int j = 0;
        bool run = true;
        while (j < numOfSzallitmany && run)
        {

            if (strcmp(tempDB[j].fajta, szallitmanyDB[i].fajta) == 0)
            {
                tempDB[j].szoloMennyiseg += szallitmanyDB[i].mennyiseg;
                run = false;
            }
            else if (strcmp(tempDB[j].fajta, "") == 0)
            {
                strcpy(tempDB[j].fajta, szallitmanyDB[i].fajta);
                tempDB[j].szoloMennyiseg = szallitmanyDB[i].mennyiseg;
                tempDBLen++;
                run = false;
            }
            j++;
        }
        i++;
    }

    i = 0;
    numOfFeldolgozas = 0;
    feldolgozasDB = calloc(numOfSzallitmany, sizeof(Feldolgozas));

    while (i < tempDBLen)
    {
        if (tempDB[i].szoloMennyiseg >= minAmount)
        {
            strcpy(feldolgozasDB[numOfFeldolgozas].fajta, tempDB[i].fajta);
            feldolgozasDB[numOfFeldolgozas].szoloMennyiseg = tempDB[i].szoloMennyiseg;
            feldolgozasDB[numOfFeldolgozas].borMennyiseg = 0;
            numOfFeldolgozas++;
        }
        i++;
    }
    free(tempDB);

    if (numOfFeldolgozas > 0)
    {
        feldolgozasDB = (Feldolgozas *)realloc(feldolgozasDB, (numOfFeldolgozas * sizeof(Feldolgozas)));
    }

    if (feldolgozasDB == NULL)
    {
        free(szallitmanyDB);
        printf("Hiba az újraméretezés közben!\n");
        exit(0);
    }

    return numOfFeldolgozas > 0;
}

void handlerReadyForProduction(int signum)
{
    printf("Az NSZT megkezdte a szőlő szállítását.\n", signum);
}

void handlerStartedProduction(int signum)
{
    printf("Az SZFÜ megkezdte a feldolgozást.\nKérem várjon...\n");
}

void processGrapes()
{
    signal(SIGUSR1, handlerReadyForProduction);
    signal(SIGUSR2, handlerStartedProduction);

    int pipeNsztToProcessing[2];
    int pipeProcessingToNszt[2];

    if (pipe(pipeNsztToProcessing) == -1 || pipe(pipeProcessingToNszt) == -1)
    {
        perror("Hiba a pipe nyitaskor!\n");
        free(szallitmanyDB);
        free(feldolgozasDB);
        exit(1);
    }

    pid_t child = fork();
    if (child < 0)
    {
        printf("Hiba a fork hívás közben!\n");
        free(szallitmanyDB);
        free(feldolgozasDB);
        exit(1);
    }

    // pipe[0] = read
    // pipe[1] = write
    // read/wire("pipe",cím,hossz)
    if (child > 0) // parent (NSZT)
    {
        close(pipeNsztToProcessing[0]); // nincs használva
        close(pipeProcessingToNszt[1]); // nincs használva


        printf("Az NSZT várakozik az SZFÜ jelzésére.\n");
        pause();
        sleep(3);

        write(pipeNsztToProcessing[1], &numOfFeldolgozas, sizeof(int));

        int i = 0;
        while (i < numOfFeldolgozas)
        {
            write(pipeNsztToProcessing[1], feldolgozasDB[i].fajta, 256);
            write(pipeNsztToProcessing[1], &feldolgozasDB[i].szoloMennyiseg, sizeof(int));
            write(pipeNsztToProcessing[1], &feldolgozasDB[i].borMennyiseg, sizeof(double));
            i++;
        }
        close(pipeNsztToProcessing[1]);

        pause();

        read(pipeProcessingToNszt[0], &numOfFeldolgozas, sizeof(int));
        i = 0;
        while (i < numOfFeldolgozas)
        {
            read(pipeProcessingToNszt[0], feldolgozasDB[i].fajta, 256);
            read(pipeProcessingToNszt[0], &feldolgozasDB[i].szoloMennyiseg, sizeof(int));
            read(pipeProcessingToNszt[0], &feldolgozasDB[i].borMennyiseg, sizeof(double));
            i++;
        }
        close(pipeProcessingToNszt[0]);

        wait(NULL);

        printAllProducts("Az NSZT átvette a bort!\nA visszakapott mennyiség: \n");
    }
    else // child (SZFÜ)
    {

        close(pipeProcessingToNszt[0]); // nincs használva
        close(pipeNsztToProcessing[1]); // nincs használva
        free(szallitmanyDB);
        free(feldolgozasDB);

        sleep(3);
        kill(getppid(), SIGUSR1);
        printf("Az SZFÜ készen áll a feldolgozásra.\n");

        read(pipeNsztToProcessing[0], &numOfFeldolgozas, sizeof(int));

        int i = 0;
        feldolgozasDB = calloc(numOfFeldolgozas, sizeof(Feldolgozas));

        while (i < numOfFeldolgozas)
        {
            read(pipeNsztToProcessing[0], feldolgozasDB[i].fajta, 256);
            read(pipeNsztToProcessing[0], &feldolgozasDB[i].szoloMennyiseg, sizeof(int));
            read(pipeNsztToProcessing[0], &feldolgozasDB[i].borMennyiseg, sizeof(double));
            i++;
        }
        close(pipeNsztToProcessing[0]);
        
        sleep(3);
        kill(getppid(),SIGUSR2);
        sleep(5);

        srand(time(NULL));
        float r;

        i = 0;
        while (i < numOfFeldolgozas)
        {
            r = (((float)rand() / RAND_MAX) * 0.2) + 0.6;
            feldolgozasDB[i].borMennyiseg = feldolgozasDB[i].szoloMennyiseg * r;
            i++;
        }


        write(pipeProcessingToNszt[1], &numOfFeldolgozas, sizeof(int));

        i = 0;
        while (i < numOfFeldolgozas)
        {
            write(pipeProcessingToNszt[1], feldolgozasDB[i].fajta, 256);
            write(pipeProcessingToNszt[1], &feldolgozasDB[i].szoloMennyiseg, sizeof(int));
            write(pipeProcessingToNszt[1], &feldolgozasDB[i].borMennyiseg, sizeof(double));
            i++;
        }

        close(pipeProcessingToNszt[1]);
        printf("Az SZFÜ befejezte a feldolgozást, viszlát!\n");
        sleep(5);

        exit(0);
    }
}

void processingMenu()
{
    bool canProcess = selectGrapesForProcessing();

    if (!canProcess)
    {
        clear();
        printf("Nincs olyan adat ami ennek a kritériumnak megfelel!\n");
        return;
    }

    printAllProducts("Az alábbi adatok felelnek meg a követelménynek: \n");
    bool wantToProcess = readUserConfirm("Biztosan szeretné feldolgozni ezeket a szállítmányokat? ");

    if(!wantToProcess)
    {
        clear();
        return;
    }

    processGrapes();

}

// 2. rész vége---------------------------------

bool mainMenu(int menuNum)
{

    switch (menuNum)
    {
    case 0:
        printMainMenu();
        break;

    case 1:
        printAllData();
        printMainMenu();
        break;

    case 2:
        printDataByOrigin();
        printMainMenu();
        break;

    case 3:
        reciveNewShipment();
        printMainMenu();
        break;

    case 4:
        modifyShippmentMenuController();
        printMainMenu();
        break;

    case 5:
        deleteShippmentMenuController();
        printMainMenu();
        break;

    case 6:
        processingMenu();
        printMainMenu();
        break;

    case 7:
        clear();
        return false;
        break;
    }

    printSep();
    return true;
}

int main()
{
    clear();

    inicialize();
    szallitmanyDB = calloc(100, sizeof(Szallitmany));
    numOfSzallitmany = loadData();

    int userChoice = 0;
    bool run = true;
    while (run)
    {
        run = mainMenu(userChoice);
        if (!run)
            continue;
        userChoice = readInt();
    }

    free(szallitmanyDB);
    free(feldolgozasDB);
    printf("Viszlát!\n");
    return 0;
}