#include "gradebook.h"
#include <string.h>

/*
 * Name: Rohan Bhagat
 */

struct Gradebook gradebook;

/**
 * Adds a new student to the gradebook and sets all the student's grades to 0.
 *
 * Updates assignment_averages and course_average based on the new grades.
 *
 * @param name The name of the student.
 * @param gtid The GTID of the student.
 * @param year The year of the student.
 * @param major The major of the student.
 * @return SUCCESS if the student is added, otherwise ERROR if the student can't
 * be added (duplicate name / GTID, no space in gradebook, invalid major).
 */
int add_student(char *name, int gtid, int year, char *major) {
  // create student entry
  struct Student newStudent;

  // throw error if a pointer is null
  if (!name || !major) {
    return ERROR;
  }

  // throw error if name or gtid is duplicated
  int duplicateFlag = search_student(name);
  if (duplicateFlag != ERROR) {
    return ERROR;
  }

  // throw error if char* major != an enum val
  if (strcmp(major, "CS") != 0 && strcmp(major, "CE") != 0 && strcmp(major, "EE") != 0 && strcmp(major, "IE") != 0) {
    return ERROR;
  } else {
    // Set the major based on the string
    if (strcmp(major, "CS") == 0) {
      newStudent.major = CS;
    } else if (strcmp(major, "CE") == 0) {
      newStudent.major = CE;
    } else if (strcmp(major, "EE") == 0) {
      newStudent.major = EE;
    } else if (strcmp(major, "IE") == 0) {
      newStudent.major = IE;
    }
  }

  // throw error if name is invalid
  if (strlen(name) > MAX_NAME_LENGTH - 1) {
    return ERROR;
  } else {
    strcpy(newStudent.name, name);
  }

  newStudent.gtid = gtid;
  newStudent.year = year;


  // set grades and average to 0
  struct GradebookEntry newEntry;
  newEntry.student = newStudent;
  for (int i = 0; i < NUM_ASSIGNMENTS; i++) {
    newEntry.grades[i] = 0;
  }
  newEntry.average = 0;

  // add student entry to gradebook struct
  // throw error if gradebook is full
  if (gradebook.size >= MAX_ENTRIES) {
    return ERROR;
  } else {
    gradebook.entries[gradebook.size] = newEntry;
  }

  // incremenet gradebook size
  gradebook.size++;

  // update assignment and course averages
  int huh = calculate_course_average();

  if (huh == ERROR) {
    return ERROR;
  }

  return SUCCESS;
}

/**
 * Updates the grade of a specific assignment for a student and updates that
 * student's average grade.
 * 
 * Ensure that the overall course averages are still up-to-date after these grade updates.
 *
 * @param name The name of the student.
 * @param assignmentType The type of assignment.
 * @param newGrade The new grade.
 * @return SUCCESS if the grade is updated, otherwise ERROR if the grade isn't (student not found).
 */
int update_grade(char *name, enum Assignment assignment_type, double new_grade) {
  // search for student
  int studentIndex = search_student(name);

  // return error if not found, else update student grade with new_grade
  if (studentIndex == ERROR) {
    return ERROR;
  } else {
    gradebook.entries[studentIndex].grades[assignment_type] = new_grade;
  }
    
  // updaate student average w/ function call
  int flag = calculate_average(&gradebook.entries[studentIndex]);
  if (flag == ERROR) {
    return ERROR;
  }

  // update overall course average w/ function call
  flag = calculate_course_average();
  if (flag == ERROR) {
    return ERROR;
  }

  // return
  return SUCCESS;
}

/**
 * Adds a new student to the gradebook and initializes each of the student's
 * grades with the grades passed in.
 *
 * Additionally, will update the overall assignment_averages and course_average
 * based on the new added student.
 *
 * @param name The name of the student.
 * @param gtid The GTID of the student.
 * @param year The year of the student.
 * @param major The major of the student.
 * @param grades An array of grades for the student.
 * @return SUCCESS if the student is added and the averages updated, otherwise ERROR if the student can't
 * be added (duplicate name / GTID, no space in gradebook, invalid major).
 */
int add_student_with_grades(char *name, int gtid, int year, char *major,
                            double *grades) {
  if (gradebook.size == MAX_ENTRIES) {
    return ERROR;
  }

  // call add student
  int flag = add_student(name, gtid, year, major);

  if (flag == ERROR) {
    return ERROR;
  }

  if (!grades) {
    return ERROR;
  }

  // update grades if cont
  memcpy(gradebook.entries[gradebook.size - 1].grades, grades, NUM_ASSIGNMENTS*sizeof(*grades));

  // calculate average
  calculate_average(&gradebook.entries[gradebook.size - 1]);

  // recalculate class averages
  calculate_course_average();

  return SUCCESS;
}

/**
 * Calculates the average grade for a specific gradebook entry and updates the
 * struct as appropriate.
 *
 * @param entry The gradebook entry for which to recalculate the average.
 * @return SUCCESS if the average is updated, ERROR if pointer is NULL
 */
int calculate_average(struct GradebookEntry *entry) {
  // calculate weighted average for student grades if pointer is valid
  if (entry) {
    double total = 0.0;
    for (int i = 0; i < NUM_ASSIGNMENTS; i++) {
      total += (gradebook.weights[i] * entry->grades[i]);
    }
    entry->average = total;
    return SUCCESS;
} else {
    return ERROR;
}
}

/**
 * Calculates and update the overall course average and assignment averages. 
 * The average should be calculated by taking the averages of the student's 
 * averages, NOT the assignment averages.
 *
 * If the gradebook is empty, set the course and assignment averages to 0
 * and return ERROR.
 * 
 * @return SUCCESS if the averages are calculated properly, ERROR if gradebook
 * is empty
 */
int calculate_course_average(void) {
  // throw ERROR if gradebook is empty and set all averages to 0
  if (gradebook.size == 0) {
    for (int i = 0; i < NUM_ASSIGNMENTS; i++) {
      gradebook.assignment_averages[i] = 0;
    }
    gradebook.course_average = 0;
    return ERROR;
  }

  // calculate average of student averages for each column
  for (int i = 0; i < NUM_ASSIGNMENTS; i++) {
    double colSum = 0.0;
    for (int j = 0; j < gradebook.size; j++) {
      colSum += gradebook.entries[j].grades[i];
    }
    gradebook.assignment_averages[i] = (colSum/gradebook.size);
  }

  // calculate course average and store in course_average
  double courseSum = 0.0;
  for (int k = 0; k < gradebook.size; k++) {
    courseSum += gradebook.entries[k].average;
  }
  gradebook.course_average = (courseSum/gradebook.size);
  
  // return SUCCESS
  return SUCCESS;
}

/**
 * Searches for a student in the gradebook by name.
 *
 * @param name The name of the student.
 * @return The index of the student in the gradebook, or ERROR if not found.
 */
int search_student(char *name) {
  if (gradebook.size == 0 || !name) {
    return ERROR;
  }

  // search for student and return index where the student is located at
  for (int i = 0; i < gradebook.size; i++) {
    if (strcmp(gradebook.entries[i].student.name, name) == 0) {
      return i;
    }
  }
  return ERROR;
}

/**
 * Remove a student from the gradebook while maintaining the ordering of the gradebook.
 *
 * Additionally, update the overall assignment_averages and course_average
 * based on the removed student and decrement the size of gradebook.
 *
 * If the gradebook is empty afterwards, SUCCESS should still be returned and
 * averages should be set to 0.
 *
 * @param name The name of the student to be withdrawn.
 * @return SUCCESS if the student is successfully removed, otherwise ERROR if
 * the student isn't found.
 */
int withdraw_student(char *name) {

  if (!name) {
    return ERROR;
  }

  // search for student
  int index = search_student(name);

  // throw error if not found
  if (index == ERROR) {
    return ERROR;
  }
  
  // remove student and remove gap in gradebook
  while (index < gradebook.size - 1) {
    gradebook.entries[index] = gradebook.entries[index+1];
    index++;
  }
  // gradebook.entries[gradebook.size - 1] = 0;

  // decrement size
  gradebook.size--;

  // update averages and course averages
  if (gradebook.size != 0) {
    calculate_course_average();
  } else {
    // if the gradebook is empty after removing the student,
    // averages should be set to 0 and success should be returned
    for (int i = 0; i < (gradebook.size); i++) {
      gradebook.assignment_averages[i] = 0;
    }
    gradebook.course_average = 0;
  }
  
  return SUCCESS;
}

/**
 * Populate the provided array with the GTIDs of the 5 students with the highest
 * grades. The GTIDs should be placed in descending order of averages. 
 * 
 * If unable to populate the full array (less than 5 students in gradebook), 
 * fill in the remaining values with INVALID_GTID.
 *
 * @param gtids An array to store the top five gtids.
 * @return SUCCESS if gtids are found, otherwise ERROR if gradebook is empty
 */
int top_five_gtid(int *gtids) {
  // throw error if gradebook is empty
  if (gradebook.size == 0) {
    return ERROR;
  }
  // fill arary with gtids of students with the highest coruse averages
  sort_averages();

  // populate rest of array with INVALID_gtid if gradebook has been exhausted
  for (int i = 0; i < 5; i++) {
    if (gradebook.size <= i) {
      gtids[i] = INVALID_GTID;
    } else {
    gtids[i] = gradebook.entries[i].student.gtid;
    }
  }
  return SUCCESS;
}

/**
 * Sorts the gradebook entries by name in alphabetical order (First, Last).
 *
 * @return SUCCESS if names are sorted, ERROR is gradebook is empty.
 */
int sort_name(void) {
  // throw error if gradebook is empty
  if (gradebook.size == 0) {
    return ERROR;
  }

  // sort gradebook in alphabetical order using bubble sort
  if (gradebook.size < 2) {
    return SUCCESS;
  }

  int stopPoint = gradebook.size - 1;
  int lastSwap = 0;
  struct GradebookEntry temp;
  while (stopPoint != 0) {
    lastSwap = 0;
    for (int i = 0; i < stopPoint; i++) {
      if (strcmp(gradebook.entries[i].student.name, gradebook.entries[i + 1].student.name) > 0) {
        temp = gradebook.entries[i];
        gradebook.entries[i] = gradebook.entries[i+1];
        gradebook.entries[i+1] = temp;
        lastSwap = i;
      }
    }
    stopPoint = lastSwap;
  }

  return SUCCESS;
}

/**
 * Sorts the gradebook entries by average grades in descending order.
 *
 * @return SUCCESS if entries are sorted, ERROR if gradebook is empty.
 */
int sort_averages(void) {
  // throw error if gradebook is empty
  if (gradebook.size == 0) {
    return ERROR;
  }
  // stable sort gradebook (using bubble sort) in descending oder by course average
  if (gradebook.size < 2) {
    return SUCCESS;
  }

  int stopPoint = gradebook.size - 1;
  int lastSwap = 0;
  struct GradebookEntry temp;
  while (stopPoint != 0) {
    lastSwap = 0;
    for (int i = 0; i < stopPoint; i++) {
      if (gradebook.entries[i].average < gradebook.entries[i + 1].average) {
        temp = gradebook.entries[i];
        gradebook.entries[i] = gradebook.entries[i+1];
        gradebook.entries[i+1] = temp;
        lastSwap = i;
      }
    }
    stopPoint = lastSwap;
  }

  return SUCCESS;
}


/**
 * Prints the entire gradebook in the format
 * student_name,major,grade1,grade2,...,student_average\n
 * 
 * Overall Averages:
 * grade1_average,grade2_average,...,course_average\n
 * 
 * Note 1: The '\n' shouldn’t print, just represents the newline for this example.
 * Note 2: There is an empty line above the “Overall Averages:” line.
 * 
 * All of the floats that you print must be manually rounded to 2 decimal places.
 *
 * @return SUCCESS if gradebook is printed, ERROR if gradebook is empty.
 */
int print_gradebook(void) {
  // throw error if gradebook is empty
  if (gradebook.size == 0) {
    return ERROR;
  }

  // print gradebook as csv with manual rounding]
  char majorAbb[3];
  for (int i = 0; i < gradebook.size; i++) {
    int majorVal = gradebook.entries[i].student.major;
    if (majorVal == 0) {
      memcpy(majorAbb, "CS", 3);
    } else if (majorVal == 1) {
      memcpy(majorAbb, "CE", 3);
    } else if (majorVal == 2) {
      memcpy(majorAbb, "EE", 3);
    } else if (majorVal == 3) {
      memcpy(majorAbb, "IE", 3);
    } else {
      return ERROR;
    }

    
    printf("%s,%s,", gradebook.entries[i].student.name, majorAbb);
    for (int j = 0; j < NUM_ASSIGNMENTS; j++) {
      printf("%.2f,", gradebook.entries[i].grades[j]);
    }
    printf("%.2f\n", gradebook.entries[i].average);
  }
  
  printf("\n");
  printf("Overall Averages:\n");
  
  for (int k = 0; k < NUM_ASSIGNMENTS; k++) {
    printf("%.2f,", gradebook.assignment_averages[k]);
  }
  printf("%.2f\n", gradebook.course_average);

  return SUCCESS;
}
