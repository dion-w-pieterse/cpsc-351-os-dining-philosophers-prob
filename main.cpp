/*
 Name: Dion Pieterse
 Class: CPSC 351
 Project: Chapter 5 - Dining Philosopher's Problem.
 Description:
 This is the Dining Philosopher's problem.
 The program ensures that there is no deadlock or starvation for any of the philosophers sitting at the table,
 when they attempt to eat by picking up the forks from their left and right side.
 */

#include <iostream>
#include <iomanip>
#include <sstream>
#include <ctype.h>
#include <ctime>
#include <unistd.h>
#include <cstdlib>
#include <pthread.h>

using namespace std;

//Prototypes:
class TheDinersTable;

//Structure used to pass arguments
struct philosophers_t {
    int numOfPhilosophers;
    int attemptsToEat;
    int haveEaten;
    TheDinersTable *table;
};


/*****************************************
 *** THE-DINERS-TABLE CLASS DEFINITION ***
 ****************************************/
class TheDinersTable {

public:

    /******************************************************************************
     *** 2 ARG CONSTRUCTOR: TheDinersTable(int numPhilosophers, int numAttempts)
     Description: Initializes the Dining table.
     *****************************************************************************/
    TheDinersTable(int numPhilosophers, int numAttempts)
    : numOfPhilosophers(numPhilosophers), attemptsToEat(numAttempts) {

        //Initialize the mutex lock for the table
        pthread_mutex_init(&diningTable_mutex, NULL);

        //Instantiate all the class arrays
        philThreadArray = new pthread_t[numOfPhilosophers];
        paramArray = new philosophers_t[numOfPhilosophers];
        arrThrdCond_ForSafeToEat = new pthread_cond_t[numOfPhilosophers];
        statusOfPhilosopher = new int[numOfPhilosophers];
        haveEatenArray = new int[numOfPhilosophers];

        //Populate all the arrays with information.
        for(int counter = 0; counter < numOfPhilosophers; ++counter) {
            haveEatenArray[counter] = 0;
            pthread_cond_init(&arrThrdCond_ForSafeToEat[counter], NULL);
            paramArray[counter].table = this;
            statusOfPhilosopher[counter] = THINKING;
            paramArray[counter].attemptsToEat = attemptsToEat;
            paramArray[counter].numOfPhilosophers = counter;
            paramArray[counter].haveEaten = 0;
        }
    }


    /******************************************************************************
     *** DESTRUCTOR: ~TheDinersTable()
     Description: Destructor to destroy all the arrays from dynamic memory.
     *****************************************************************************/
    ~TheDinersTable() {
        delete [] paramArray;
        delete [] statusOfPhilosopher;
        delete [] arrThrdCond_ForSafeToEat;
        delete [] philThreadArray;
        delete [] haveEatenArray;
    }

    /******************************************************************************
     *** BEGIN DINNER: dinnerIsServed()
     Description: Simulates everyone beginning dinner.
     *****************************************************************************/
    void dinnerIsServed() {
        //Print the column titles.
        printColTitles();

        //Print the states of every person at the table in the beginning.
        showStatusOfPeople();

        //Make pthreads for each person at the table and pass the runner function: bonAppetite with its argument structure from the argument array.
        for (int counter = 0; counter < numOfPhilosophers; ++counter)
        {
            pthread_create(&philThreadArray[counter], NULL, bonAppetite, &paramArray[counter]);
        }
        //Join all pthreads once they return from completion.
        for (int counter = 0; counter < numOfPhilosophers; ++counter)
        {
            pthread_join(philThreadArray[counter], NULL);
        }

        cout << "All Finished." << endl;
    }

    /******************************************************************************
     *** PUT DOWN FORKS: putDownForks(int person)
     Description: Simulates a person putting down the forks when done.
     *****************************************************************************/
    void putDownForks(int person) {

        //Set the mutex lock to safe guard resources.
        pthread_mutex_lock(&diningTable_mutex);
        //Now person is done eating, set them to the thinking state.
        statusOfPhilosopher[person] = THINKING;

        //Verify the status of people next to the person and let them know if they can eat safely.
        if (safeToEat(personOnLeft(person)) == true) {
            //It is safe for person of left so change their cond variable value to reflect safety.
            pthread_cond_signal(&arrThrdCond_ForSafeToEat[personOnLeft(person)]);
        }
        if (safeToEat(personOnRight(person)) == true) {
            //It is safe for person on right to eat, so change the condition variable to reflect safety.
            pthread_cond_signal(&arrThrdCond_ForSafeToEat[personOnRight(person)]);
        }

        //unlock the mutex when method is complete.
        pthread_mutex_unlock(&diningTable_mutex);
    }


    /******************************************************************************
     *** PICK UP FORKS: pickupForks(int person)
     Description: Simulates a person picking up forks.
     *****************************************************************************/
    void pickupForks(int person) {
        //Set the mutex lock to guard resources.
        pthread_mutex_lock(&diningTable_mutex);
        //Set the state to hungry.
        statusOfPhilosopher[person] = HUNGRY;

        //determine if the person can actually eat now.
        if (safeToEat(person) == false) {
            //Make the pthread wait until it is safe to eat.
            pthread_cond_wait(&arrThrdCond_ForSafeToEat[person], &diningTable_mutex);
        }
        //unlock the mutex after all done.
        pthread_mutex_unlock(&diningTable_mutex);
    }

private:

    //Array holding the state of each person at the table.
    int* statusOfPhilosopher;

    //The number of people at the table.
    int numOfPhilosophers;

    //The number of times each person tries to eat food.
    int attemptsToEat;

    //Array for the number of have eatens
    int* haveEatenArray;

    //Array of parameter structures that are passed as arguments for the runner function of each thread.
    philosophers_t* paramArray;

    //Array of pthreads for each of the 5 people at the table.
    pthread_t* philThreadArray;

    //Mutex lock to guard resources for the runner function: bonAppetite.
    pthread_mutex_t diningTable_mutex;

    //Array of conditions to ensure people don't eat unless the can safely eat.
    pthread_cond_t* arrThrdCond_ForSafeToEat;

    //The different states that each philosopher can be in at the table.
    enum { THINKING, HUNGRY, EATING };

    //The positions of all the philosophers at the table
    enum {POS_0, POS_1, POS_2, POS_3, POS_4};


    /************************************************************************************************
     *** DISPLAY TABLE COLUMN TITLES: printColTitles()
     Description: Prints the columns titles.
     ************************************************************************************************/
    void printColTitles() {
        cout << left;
        cout << "=====================================================================" << endl;
        cout << "|                      === PHILOSOPHERS ===                         |" << endl;
        cout << "=====================================================================" << endl;
        for(int person = 0; person < numOfPhilosophers; ++person) {
            cout << setw(8) <<"   Phil_" << setw(3) << left << (person+1);
            if(person < numOfPhilosophers) {
                cout << " | ";
            }
        }
        cout << "\n";
        cout << "---------------------------------------------------------------------" << endl;
    }


    /************************************************************************************************
     *** SHOW STATUS OF ALL PEOPLE: showStatusOfPeople()
     Description: Shows the status of all the people at the table.
     ************************************************************************************************/
    void showStatusOfPeople()
    {
        for( int person = 0; person < numOfPhilosophers; ++person) {
            switch(statusOfPhilosopher[person]) {
                case THINKING: cout << setw(12) << left << "thinking" << setw(2) << left << "|";
                    break;
                case HUNGRY: cout << setw(12) << left << "*Hungry*" << setw(2) << left << "|";
                    break;
                case EATING:
                    stringstream ss;
                    ss << "EATING (" << haveEatenArray[person] << ")";
                    cout << setw(12) << ss.str() << setw(2) << left << "|";
                    break;
            }
        }
        cout << "\n";
        cout << "---------------------------------------------------------------------\n";
    }


    /************************************************************************************************
     *** TABLE POSITION OF PERSON ON LEFT: personOnLeft(int person)
     Description: Return the table position (0-4) of person on left of person parameter.
     ************************************************************************************************/
    int personOnLeft(int person) {  return (person == POS_4 ? POS_0 : person + 1); }


    /************************************************************************************************
     *** TABLE POSITION OF PERSON ON RIGHT: personOnRight(int person)
     Description: Return the table position (0-4) of person on right of person parameter.
     ************************************************************************************************/
    int personOnRight(int person) {
        if(person == POS_0) {
            return POS_4;
        }
        else {
            return (person - 1);
        }
    }


    /******************************************************************************
     *** CHECKING IF SAFE TO EAT METHOD: safeToEat(int person)
     Description: Checks if it is safe for the person to eat.
     *****************************************************************************/
    bool safeToEat(int person) {
        //checks if current philosopher's state is HUNGRY
        if (statusOfPhilosopher[person] != HUNGRY) { return false;  }

        if (statusOfPhilosopher[personOnLeft(person)] == EATING || statusOfPhilosopher[personOnRight(person)] == EATING) { return false; }

        statusOfPhilosopher[person] = EATING;
        ++haveEatenArray[person]; //increment person has eaten
        showStatusOfPeople();

        return true;
    }


    /******************************************************************************
     *** TIME EATING METHOD: personEating()
     Description: Sleeps for some time to represent a person eating.
     *****************************************************************************/
    static void personEating() {
        usleep(rand() % 1000000);
    }


    /******************************************************************************
     *** TIME EATING METHOD: personThinking()
     Description: Sleeps for some time to represent a person thinking.
     *****************************************************************************/
    static void personThinking() {
        usleep(rand() % 1000000);
    }


    /******************************************************************************
     *** RUNNER METHOD: bonAppetite(void* p)
     Description: This method is run by each thread for each person at the table.
     *****************************************************************************/
    static void* bonAppetite(void* p) {

        philosophers_t* argument = static_cast<philosophers_t *>(p);

        for(int attemptCount = 0; attemptCount < argument->attemptsToEat; ++attemptCount) {
            personThinking();
            argument->table->pickupForks(argument->numOfPhilosophers);
            personEating();
            argument->table->putDownForks(argument->numOfPhilosophers);
        }
        return NULL;
    }
};


/*************************************
 *** MAIN FUNCTION ***
 ************************************/
int main(int argc, char* argv[]) {
    //establish a seed based on time and set srand to seed so different each time you run rand()
    unsigned long seed = time(0);
    srand(static_cast<int>(seed));

    //Make sure there are 2 arguments passed when running the program, otherwise throw an error.
    if(argc < 2) {
        cerr << "Two arguments are required: # of philosophers and # of times each philosopher attempts to eat." << endl;
        exit(1);
    }

    //Store the values passed as command line arguments for the number of people and how many times they eat.
    int numPhilosophers = atoi(argv[1]);
    int attemptsToEat = atoi(argv[2]);

    //Instantiate the dinner table
    TheDinersTable theTable(numPhilosophers, attemptsToEat);

    //Begin the process among the philosophers
    theTable.dinnerIsServed();
    return 0;
}
