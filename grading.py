import json
import os
import subprocess
import traceback
import sys
import signal

grades = {'marks': []}


def signal_handler(signal, frame):
  print('You pressed Ctrl+C!')
  print("\n"*4, "Exiting\n", grades)
  sys.exit(0)


signal.signal(signal.SIGINT, signal_handler)

Assignment_Max_Grade = {
    "group": "g-grouptwo",
    "documentationcommenting": {"mark": 2.00, "comment": "Comment on Documentation/Commenting."},
    "designstyleimplementation": {"mark": 15.00, "comment": "Comment on Design/Style/Implementation."},
    "execution": {"mark": 13.00, "comment": "Comment on Execution."},
    "late_percent": 0.0,
    "mark_penalty": 0.0,
    "mark_penalty_reason": "Reason for mark penalty",
    "overall_comment": "Overall comment on the assignment."
}

ROOT_PATH = os.getcwd()
submissionPath = "{}/submissions/".format(ROOT_PATH)
print(submissionPath)
listOfSumbissions = sorted([f.path for f in os.scandir(submissionPath) if f.is_dir()])
# os.getcwd()
print("\n"*3)
try:
  for i in range(len(listOfSumbissions)):
    groupId = listOfSumbissions[i].split('/')[-1]

    Assignment_Max_Grade = {
        "group": groupId,
        "documentationcommenting": {"mark": 0.00, "comment": "Comment on Documentation/Commenting."},
        "designstyleimplementation": {"mark": 0.00, "comment": "Comment on Design/Style/Implementation."},
        "execution": {"mark": 0.00, "comment": "Comment on Execution."},
        "late_percent": 0.0,
        "mark_penalty": 0.0,
        "mark_penalty_reason": "Reason for mark penalty",
        "overall_comment": "Overall comment on the assignment."
    }
    os.chdir(listOfSumbissions[i])
    currentDir = (os.getcwd())
    # print(currentDir)

    getFiles = "grep -irn '' --include '*.zip'"
    outputOfGetFiles = subprocess.Popen(getFiles, shell=True, stdout=subprocess.PIPE,
                                        stderr=subprocess.STDOUT).stdout.read()
    extractedFolder = outputOfGetFiles.decode('utf-8').split()[2]
    # print(outputOfGetFiles.decode('utf-8').split())
    # print("Output Folder: {}".format(extractedFolder))

    checkIfDirectoryExistCmd = "unzip -qql {}  | head -n1 | tr -s ' ' | cut -d' ' -f5-".format(
        extractedFolder)
    outputOfTarCommand = subprocess.Popen(checkIfDirectoryExistCmd, shell=True, stdout=subprocess.PIPE,
                                          stderr=subprocess.STDOUT).stdout.read()
    extractedFolderOrNoFolder = outputOfTarCommand.decode('utf-8').strip()
    # print(outputOfTarCommand.decode('utf-8'))
    # print("Output Folder: {}".format(extractedFolder))

    if(extractedFolderOrNoFolder[-1] == '/'):
      # update the path to list of submissions

      finalSubmissionDirPath = "{}/{}".format(listOfSumbissions[i], extractedFolderOrNoFolder)
      print("currentDir: {} | Directory: {}".format(
          finalSubmissionDirPath, extractedFolderOrNoFolder))
    else:
      finalSubmissionDirPath = listOfSumbissions[i]
      print("currentDir: {} | File: {}".format(finalSubmissionDirPath, extractedFolderOrNoFolder))
    # Actually Extract all
    unzipFilesCommand = "unzip -o {}".format(
        extractedFolder)
    unzipFilesCommandOutput = subprocess.Popen(unzipFilesCommand, shell=True, stdout=subprocess.PIPE,
                                               stderr=subprocess.STDOUT).stdout.read().decode('utf-8').strip()

    while True:
      try:
        print("\n\n\tGrading GROUP: [ {} ]".format(groupId))
        grades = int(
            input("\n\tGrade [ DOCUMENTATION & COMMENTING ] part of Code. Input [0 - 2] : "))
        Assignment_Max_Grade["documentationcommenting"]['mark'] = grades
        inputComment = input("\tComments? [string]: ")
        Assignment_Max_Grade["documentationcommenting"]['comment'] = inputComment

        grades = int(input("\n\tGrade [ DESIGN & IMPLEMENTATION ] part of Code. Input [0 - 15] : "))
        Assignment_Max_Grade["designstyleimplementation"]['mark'] = grades
        inputComment = input("\tComments? [string]: ")
        Assignment_Max_Grade["designstyleimplementation"]['comment'] = inputComment

        grades = int(input("\n\tGrade [ EXECUTION ] part of Code. Input [0 - 13] : "))
        Assignment_Max_Grade["execution"]['mark'] = grades
        inputComment = input("\tComments? [string]: ")
        Assignment_Max_Grade["execution"]['comment'] = inputComment

        inputComment = input("\n\nADD OVERALL COMMENT. Input [STRING] : ")
        Assignment_Max_Grade["overall_comment"] = inputComment
        print('\n\n\t\t', Assignment_Max_Grade, '\n')
        print("*"*80, '\n\n')
        break
      except ValueError:
        print("\n\n\t\t\t!!!!!!!!aTRY AGAIN")
        continue

  # print(os.getcwd())
  # output = subprocess.Popen("make", shell=True, stdout=subprocess.PIPE,
  #                           stderr=subprocess.STDOUT).stdout.read()
  # print(output.decode('utf-8'))

  # os.chdir(currentDir)
  # print(os.getcwd())
except Exception:

  os.chdir(ROOT_PATH)
  traceback.print_exc()
