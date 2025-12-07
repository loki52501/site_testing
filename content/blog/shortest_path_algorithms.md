# Shortest path algorithms

Dijkstra Algorithm , bellman ford algorithm and floyd warshall algorithm

# Dijkstra Algorithm

1. we are gonna look at Dijkstra Algorithm:

this algorithm gives us a pathway to find the shortest path provided there are no negative weights between source and destination. I could use a diagram to show you this.

![shortest path.jpg](../images/shortest%20path.jpg)

we are going to visit each node and see if there is a shortest path from jack’s place to new york using Dijkstra’s algorithm. 

we will be traversing each node and see the shortest path to the next node or to the destination.

the iteration starts from jack’s place and his adjacent nodes,

jack’s place value is initialized to zero.

we will check if u(i)+weight<\v , where i is the current value of the path, and weight is the value from that path to v, where v is the destination. if there is a v already and if the current weight path is lesser than that, then v needs to be updated, as shortest path to v. we will track each node via visited boolean variable, that keeps track of each node, if it was visited or not.

# First Iteration

1. when we start, u1 will be jack, and from his place, we will calculate all the possible destinations and it's weights. initally all values will be assigned infinity, if there is a value already, we will compare it with the new value and if it is small then we will replace. there will be a visited variable which tracks all the values we have visited. so the visited of jacks place is set to true in the first iteration.

| possible v value | philly | sometown | new jersey | nyc |
| --- | --- | --- | --- | --- |
| u1 | <span style="background-color: #90EE90;">5</span> | inf | inf | 26 |

# Second iteration

1. In second iteration we know the values that we got from the first iteration, the next path that will be choosen here is the shortest path from u1, i.e philly, so u2, will be philly, from u2 we will figure out all the possible values . philly will be marked as visited, so that there is no need to visit philly again.

| possible v value | philly | sometown | new jersey | nyc |
| --- | --- | --- | --- | --- |
| u1 | <span style="background-color: #90EE90;">5</span> | inf | inf | 26 |
| u2 | <span style="background-color: #90EE90;">5</span> | <span style="background-color: #FFD700;">14</span> | <span style="background-color: #FFD700;">13</span> | 26 |

# Third iteration

1. In third iteration, we will chose our u3 value as new jersey, as it is shortest from philly, and it will be marked as visited. now we will calculate the shortest values, by the if condition(u2+weight<\v) where u3=u2+weight, and v is the preexisting value at the destination.

| possible v value | philly | sometown | new jersey | nyc |
| --- | --- | --- | --- | --- |
| u1 | <span style="background-color: #90EE90;">5</span> | inf | inf | 26 |
| u2 | <span style="background-color: #90EE90;">5</span> | <span style="background-color: #FFD700;">14</span> | <span style="background-color: #FFD700;">13</span> | 26 |
| u3 | <span style="background-color: #90EE90;">5</span> | <span style="background-color: #90EE90;">14</span> | <span style="background-color: #90EE90;">13</span> | <span style="background-color: #FFD700;">17</span> |

nyc value gets update as the previous value is larger than the current weight value. 

# Fourth Iteration

since sometown hasn't been visited yet, we go to sometown, with it's current path value of 14, and will be marked as visited.

| possible v value | philly | sometown | new jersey | nyc |
| --- | --- | --- | --- | --- |
| u1 | <span style="background-color: #90EE90;">5</span> | inf | inf | 26 |
| u2 | <span style="background-color: #90EE90;">5</span> | <span style="background-color: #FFD700;">14</span> | <span style="background-color: #FFD700;">13</span> | 26 |
| u3 | <span style="background-color: #90EE90;">5</span> | <span style="background-color: #90EE90;">14</span> | <span style="background-color: #90EE90;">13</span> | <span style="background-color: #FFD700;">17</span> |
| u4 | <span style="background-color: #90EE90;">5</span> | <span style="background-color: #90EE90;">14</span> | <span style="background-color: #90EE90;">13</span> | <span style="background-color: #FFD700;">16</span> |

Here nyc value gets update as there is a shortest path from sometown . 

# Bellman Ford Algorithm

This algorithm is similar to that of dijkstra. but bellman ford helps detecting negative cycles. bellman ford, doesn't keep track of visited nodes, rather.