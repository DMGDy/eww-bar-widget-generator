# eww widget Bar written in C

A simple program that generates yuck code to have a dynamic bar widget to keep track of workspaces.
It also supports clicking on the workspace as they are all buttons to change to the clicked workspace.

this is meant for the hyprland compositor.

C is really fast. This probably outperforms scripts in Python or other scripting languages 
while providing 3 different scss classes to customize.

Theres no good reason to use C instead of another program if CPU usage is not an issue. I thought 
this would be fun to do.

* `active`
    * currently active workspace
* `inactive`
    * workspaces with open windows but not focused
* `empty`
    * workspaces with nothing in them

The program is idle and only runs once there is an event related to workspaces that occur 
on one of hyprland's available socket. Additionally, .yuck code is only produced if
there is a change between the last event. 

<figure>
    <img src="./ss.png"/>
    <figcaption>
    How my bar is styled. heavily inspired by dwm's default bar
    </figcaption>
</figure>

