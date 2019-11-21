# Blackjack
# Created by Nick Martin, Isaiah Parrie, Chi Shing Poon, Dylan Chan, And Sunny Rajbhandari
cd src
make
./dealer 9000
open another terminal window in the same directory
./player 127.0.0.1

Enter a game of Blackjack! 


Things to Note:
If playing in multiplayer mode, there hasnt been any mutex added for input/output threads so its necessary to 
adding empty newlines after a command to refresh the terminal
