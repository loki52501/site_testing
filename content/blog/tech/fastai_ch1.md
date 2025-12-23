# Deep Learning Q&A Notes

## Applications and Fundamentals

### Five Areas Where Deep Learning Excels
Deep learning is now the best tool in the world for:
- Detecting anomalies in cybersecurity
- Medicine
- Self-driving cars
- Natural language processing
- Robotics and recommendation systems

### Historical Context

**Q: What was the name of the first device based on the principle of the artificial neuron?**  
A: Mark 1 Perceptron

**Q: What are the requirements for Parallel Distributed Processing (PDP)?**  
Based on the book of the same name:
- A set of processing units
- A state of activation
- An output function for each unit
- A pattern of connectivity among units
- A propagation rule for propagating patterns of activities through the network of connectivities
- An activation rule for combining the inputs impinging on a unit with the current state of that unit to produce an output for the unit
- A learning rule whereby patterns of connectivity are modified by experience
- An environment within which the systems must operate

**Q: What were the two theoretical misunderstandings that held back the field of neural networks?**  
A: It was hard to calculate the approximation even if added few layers.

## Hardware and Computing

**Q: What is a GPU?**  
A: A Graphics Processing Unit that uses SIMD (Single Instruction, Multiple Data) architecture. It can handle thousands of single tasks at the same time. GPUs can run neural networks hundreds of times faster than regular CPUs.

**Q: Open a notebook and execute a cell containing: 1+1. What happens?**  
A: Returns `2`

## Core Concepts

### Model Understanding

**Q: Why is it hard to use a traditional computer program to recognize images in a photo?**  
A: It works differently from the brain.

**Q: What did Samuel mean by "weight assignment"?**  
A: The parameters or features of a model - a variable in a mathematical equation that determines the output according to the relevant input.

**Q: What term do we normally use in deep learning for what Samuel called "weights"?**  
A: Parameters

### Samuel's Machine Learning Model

> "Suppose we arrange for some automatic means of testing the effectiveness of any current weight assignment in terms of actual performance and provide a mechanism for altering the weight assignment so as to maximize the performance. We need not go into the details of such a procedure to see that it could be made entirely automatic and to see that a machine so programmed would 'learn' from its experience."

![Samuel's ML Model Diagram](../assets/dlcf_0106_1766443982396_0.png)

**Q: Why is it hard to understand why a deep learning model makes a particular prediction?**  
A: Because it does a lot of calculations inside the neurons that it's hard to predict how it works.

### Theoretical Foundations

**Q: What is the name of the theorem that shows that a neural network can solve any mathematical problem to any level of accuracy?**  
A: Universal Approximation Theorem

**Q: What do you need in order to train a model?**  
A: Data - lots of data.

## Practical Considerations

### Bias and Ethics

**Q: How could a feedback loop impact the rollout of a predictive policing model?**  
A: It can make the model biased through more positive feedback, creating a problematic loop.

### Image Processing

**Q: Do we always have to use 224×224-pixel images with the cat recognition model?**  
A: Not necessary. If you use lower pixels, it gets more difficult, but if you increase the size, it may be much more accurate but at the price of memory consumption and speed.

**Q: What is the difference between classification and regression?**  
- **Classification**: Predicts an entity to a known label which it was trained on during training
- **Regression**: Predicts a numerical outcome, such as temperature or a location

## Training and Validation

### Datasets

**Q: What is a validation set? What is a test set? Why do we need them?**

**Validation Set**: A part of 20% of a dataset that is used to verify if the model that is being trained using the training set (80% of the dataset) correctly predicts the logical values.

**Test Set**: Contains data that's been untouched by humans as well as the model - it's from a completely different dataset. It's needed for validating that the model is producing output as it intended to do.

**Why both?** With the validation set, one can never be sure if the model has seen it or not. Even if the model never sees it, we see it, and we change hyperparameters such that the weights will closely align with the validation sets, making it less reliable.

**Q: What will fastai do if you don't provide a validation set?**  
A: Fastai will create a validation set on its own.

**Q: Can we always use a random sample for a validation set? Why or why not?**  
A: 20% is the usual proportion used.

### Overfitting

**Q: What is overfitting? Provide an example.**  
A: Overfitting tends to occur when the model memorizes instead of creating a function to predict or classify, by which means it cannot predict about an unknown quantity that is not present in its training set.

**Example**: If you give all images of an orange cat, and you ask it to predict if a grey colored cat is a cat, it's going to return false.

### Metrics and Loss

**Q: What is a metric? How does it differ from loss?**
- **Metric**: A value that determines how well the model performs against a validation set overall, designed for human consumption
- **Loss**: A measure of how good the model is, chosen to drive training via SGD (Stochastic Gradient Descent) for the neural networks

## Model Architecture

### Transfer Learning

**Q: How can pretrained models help?**  
A: Pretrained models have already been trained and can be used to further train using our data to fine-tune our answers.

**Q: What is the "head" of a model?**  
A: The part that is newly added to be specific to the new dataset.

### CNN Feature Hierarchy

**Q: What kinds of features do the early layers of a CNN find?**

**Early Layers:**
- Simple, low-level features: edges, gradients, colors, basic textures
- Very generic patterns that appear in almost any image
- Small receptive fields (look at tiny patches)

**Middle Layers:**
- Combinations of those simple features
- Textures, corners, curves, simple shapes
- Starting to detect patterns like "circles," "grids," "repeating patterns"

**Later/Deeper Layers:**
- High-level, semantic features specific to your task
- Object parts: eyes, wheels, fur, windows
- Complex patterns: "dog face," "car body," "building facade"
- Task-specific concepts

**The Progression:**
- Early: edges → corners → textures
- Middle: shapes → patterns → object parts
- Late: whole objects → semantic concepts

**Example in Practice:**
- Layer 1: horizontal/vertical edges
- Layer 5: wheel-like circles, fur texture
- Layer 15: "this looks like a dog's face"

**Key Insight**: The network builds a hierarchy - simple → complex. Each layer uses the previous layer's features as building blocks.

### Architecture Concepts

**Q: Are image models useful only for photos?**  
A: No, they can also be used for anomaly detection and other applications.

**Q: What is an architecture?**  
A: The template of the model that we're trying to fit - the actual mathematical function that we're passing the input data and parameters to.

**Q: What is segmentation?**  
A: Creating a model that can recognize every pixel of an image.

### Hyperparameters

**Q: What is y_range used for? When do we need it?**  
A: `y_range` is used for continuous outputs to constrain where our target should range. It's a constraint that the model should follow such that its output should lie between these values. You need it when your target has known bounds like ratings, and you want to prevent nonsensical predictions.

**Q: What are hyperparameters?**  
A: Hyperparameters are used to determine weights or parameters. They can include activation functions, learning rate/step, and other factors necessary to influence the parameters of a neural network.

## Best Practices

**Q: What's the best way to avoid failures when using AI in an organization?**  
A: Have test data for an AI such that it will help avoid failures.

---

*These notes appear to be from a deep learning course or textbook study guide, covering fundamental concepts in machine learning and neural networks.*