
The application is a JUCE AudioApplication which can be executed directly either using a Windows or an iOS machine. The idea 
behind the project is to be able to make an application that can play the sounds of a traditional Chinese instrument "ErHu". 

Dataset
Here is the google drive link to the dataset used for the project :

https://drive.google.com/drive/folders/1lXmBOU22nJRxse_OzvI1ku-IsEnb1y78?usp=sharing 

Place the files in the "Resources" folder in the current working directory to be able to play the audio files.

Usage

The instrument can be played using two techniques namely-
1. Plucking
2. Bowing

The user can select one of the techniques using  the radio buttons shown on GUI. Moreover, there are two ways of playing the 
audio files. 
1. Using the strings
2. Using the MIDI interface

The slider is used to control the velocity of the bow. Increasing the value of the slider will increase the velocity of the 
bowing. Another part that has been implemented is the 'flanging' effect. Clicking the flanger button opens up a window using 
which one can change the flanger parameters to be used for the playing.