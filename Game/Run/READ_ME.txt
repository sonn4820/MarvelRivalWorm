/////////////////////////////////////////////////////////////////////
How to play
/////////////////////////////////////////////////////////////////////

WS to change angle
AD to  move left and right
QE to  fly up and down
Hold space and release to adjust force and shoot
1 to use skill 1
2 to use skill 2
R to use Ultimate
P to pass the current turn

/////////////////////////////////////////////////////////////////////
How to debug 
/////////////////////////////////////////////////////////////////////
Open dev console, type "debug" to active debug mode

F1 to toggle debug draw
F2 to toggle destructible with mouse
F3 to make character move faster
F4 to fill up the ultimate bar of the current character
F5 to toggle flying of the current character
F6 to make the current character's stamina unlimited

/////////////////////////////////////////////////////////////////////
Dev Console Commands
/////////////////////////////////////////////////////////////////////

Demo: toggle AI Demo and PVP mode
Next: force skip the current turn
Restart: restart the game
test: execute commands in Commands.xml
DebugDraw: toggle debug draw
DebugDestructible:toggle destructible with mouse
DebugFast: make character move faster

//------------------------------------
For character:
(Hela, Adam, Thor, Hulk, Iron, IW)
Name: to determine what character u want to set
hp=<value>: to set health
f=<value>: to set fury
s=<value>: to set stamina
atk=<value>: to set attack
fly: to toggle fly

Example: IW hp=20 f=100 fly -> Make invisible woman's health to 20, fury to 100, and toggle fly
//------------------------------------
For AI:
(HelaAI, AdamAI, ThorAI, HulkAI, IronAI, IWAI)
Name:  to determine what character's AI u want to set (for now doesn't support 2 different AI same character)
skill=<value>: to set the skill level
learn=<true/false>: to set increment of AI skill over time

Example: IronAI skill=5 learn=false -> Make Iron man's AI skill to 5 and can't increase skill over time
