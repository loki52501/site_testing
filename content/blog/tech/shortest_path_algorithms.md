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

This algorithm is similar to that of Dijkstra, but it could detect negative cycles. that is weight with negative values forming a unending chain of loop, since the values will be one smaller than the other and so on. 

we will update the shortest distance value from u to v, if it’s lesser than the previous value. In other words, it relaxes edges.

Here we loop through n-1 times, where n is the number of vertices. why do we loop n-1 times, we can be sure that when we loop for n-1 times, a shortest path from graph with n nodes can use at most n-1 edges, as n edges would imply a cycle and cycles don’t help unless negative. 

So by repeatedly relaxing edges n-1 times, bellman ford ensures that the shortest path “propagates” through the graph fully.

![bellmanford.jpg](../images/bellmanford.jpg)

For the above graph we want to find the cheapest flight route from A to E, the flights have refunds to.. where it could be applied as rebate flight. 

we can have edges as 

A→B(5)

B→C(-5)

C→D(3)

C→E(1)

D→E(2)

We have to traverse through the entire edges n-1 times where n is the no of vertices. 

initially we start at the source, which , here is A. and assign it’s distance as 0. same as we did in dijkstra.

now we will do n-1 iterations:, here u is the src, and v is the dest, since we’re doing it for all the edges it would be O((V-1)*E), where V is no of vertices and E is no of edges.

# First Iteration

We process all edges in sequence and update distances when we find a shorter path.

**Initial state:** A=0 (source), all others = ∞

### Edge A→B (weight: 5)

A is 0, B is ∞. Calculate: 0 + 5 = 5 < ∞, so update B to 5.

### Edge B→C (weight: -5)

B is 5, C is ∞. Calculate: 5 + (-5) = 0 < ∞, so update C to 0.

### Edge C→D (weight: 3)

C is 0, D is ∞. Calculate: 0 + 3 = 3 < ∞, so update D to 3.

### Edge C→E (weight: 1)

C is 0, E is ∞. Calculate: 0 + 1 = 1 < ∞, so update E to 1.

### Edge D→E (weight: 2)

D is 3, E is 1. Calculate: 3 + 2 = 5, but 5 > 1 (current E), so no update.

| vertex | A | B | C | D | E |
| --- | --- | --- | --- | --- | --- |
| iteration 1 | <span style="background-color: #90EE90;">0</span> | <span style="background-color: #FFD700;">5</span> | <span style="background-color: #FFD700;">0</span> | <span style="background-color: #FFD700;">3</span> | <span style="background-color: #FFD700;">1</span> |

# Second Iteration

We process all edges again to check if any paths can be improved.

### Edge A→B (weight: 5)

A is 0, B is 5. Calculate: 0 + 5 = 5, which equals B's current value. No update.

### Edge B→C (weight: -5)

B is 5, C is 0. Calculate: 5 + (-5) = 0, which equals C's current value. No update.

### Edge C→D (weight: 3)

C is 0, D is 3. Calculate: 0 + 3 = 3, which equals D's current value. No update.

### Edge C→E (weight: 1)

C is 0, E is 1. Calculate: 0 + 1 = 1, which equals E's current value. No update.

### Edge D→E (weight: 2)

D is 3, E is 1. Calculate: 3 + 2 = 5, but 5 > 1 (current E). No update.

**Result:** No changes in iteration 2, meaning we've found the shortest paths!

| vertex | A | B | C | D | E |
| --- | --- | --- | --- | --- | --- |
| iteration 1 | <span style="background-color: #90EE90;">0</span> | <span style="background-color: #FFD700;">5</span> | <span style="background-color: #FFD700;">0</span> | <span style="background-color: #FFD700;">3</span> | <span style="background-color: #FFD700;">1</span> |
| iteration 2 | <span style="background-color: #90EE90;">0</span> | <span style="background-color: #90EE90;">5</span> | <span style="background-color: #90EE90;">0</span> | <span style="background-color: #90EE90;">3</span> | <span style="background-color: #90EE90;">1</span> |

The algorithm would continue for iterations 3 and 4 (since n=5 vertices, we do n-1=4 iterations), but no further updates would occur. The shortest path from A to E is **1** (via A→B→C→E). 