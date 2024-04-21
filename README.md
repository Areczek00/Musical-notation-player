Project was created, built and run using Keil MDK µVision IDE.
This is a note music player. The user writes a track, using UART: a name of the track, tempo, and notes encoded in a given format, for example: C044 D044 E044 
Writing notes ends with pressing ENTER key. Each note has to consist of 4 characters and space. First character is a pitch class: (A,B,C,D,E,F,G) or (A,H,C,D,E,F,G), second is a indicator of semitone (0, # or b), third is an octave level, forth is duration (1 is a full note, 2 is a half note, 4 is a quarter note, 8 is a eighth note, 9 is a sixteenth note).
After writing a track, user may play it. User interface is implemented with a LCD screen and a Joystick.
The project is written for Open1768 development board. Project was written in Polish language and for a Polish customer.

General description of the project is available in file 'Musical Notation Player — documentation and description.pdf'.
A simple presentation of the musical pieces being played are in videos linked from 'Presentations.txt'.
