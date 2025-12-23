<!-- content/projects.md -->

# Projects

A collection of projects demonstrating **security engineering**, **AI/ML systems**, and **infrastructure automation** skills.

---

## EdgeStreamAI — Intelligent Operations Platform for Live Streaming
*Master's Capstone Project | In Progress (Jan - May 2026)*

**Overview**

Building a hybrid intelligence platform for Netflix-scale live streaming events to help engineering teams manage high-pressure situations during major events (FIFA World Cup, Super Bowl, etc.).

**Problem**

During Netflix's Jake Paul vs Mike Tyson fight (65M concurrent streams), 30-40 engineers were in a "launch room" triaging issues in real-time. They needed to make complex decisions quickly while processing massive amounts of telemetry data from 6,000 global edge locations.

**Solution**

A two-layer system combining algorithmic edge decisions with AI-powered team assistance:

**Edge Layer (Algorithmic)**
- Quality optimization: Adaptive bitrate selection, buffer management
- Routing: Traffic distribution across CDN edges
- Security: DDoS detection, rate limiting
- Target: <5ms decision latency

**AI Layer (Gemini 3 API)**
- Natural language operations queries: "What's causing latency spike in EU West?"
- Real-time incident analysis and root cause suggestions
- Capacity prediction: "Do we have enough resources for kickoff?"
- Automated postmortem generation

**Security Research Component**
- Adversarial attack testing: Can malicious prompts manipulate AI recommendations?
- Prompt injection defense strategies
- Security benchmarking for AI in operations environments

**Tech Stack**
- Python, Gemini 3 API, NetworkX (network simulation)
- Streamlit (dashboard), SQLite (telemetry storage)
- Network simulation tools for testing at scale

**What I'm Learning**
- System design at massive scale (200M+ concurrent users)
- Trade-offs between real-time algorithmic decisions vs AI-assisted analysis
- Security considerations when deploying AI in critical infrastructure
- How to simulate and test distributed systems behavior

**Status**: Architecture complete, beginning implementation January 2026

**Inspiration**: Netflix's "Behind the Streams: Live at Netflix" blog series

---

## Network Intrusion Detection System
*ML-Powered Network Security | July 2024*

**Overview**

Built an anomaly detection system for network security using deep learning on the UNSW-NB15 dataset (real network traffic with labeled attacks).

**Approach**

Two-stage feature selection + LSTM Autoencoder:

1. **Stage 1 - Feature Selection**: Used metaheuristic algorithms (Sine Cosine Algorithm) to reduce 42 network features to ~20 most important ones
2. **Stage 2 - Refinement**: Applied Particle Swarm Optimization for final feature set optimization
3. **LSTM Autoencoder**: 
   - Encoder: Compress network traffic patterns to 4-dimensional latent space
   - Decoder: Reconstruct normal traffic
   - High reconstruction error = anomaly = potential attack

**Architecture**
```
Input (network features) 
  → LSTM(16) → LSTM(4) [bottleneck] 
  → RepeatVector 
  → LSTM(4) → LSTM(16) 
  → Output (reconstructed features)
```

**Results**
- 80% accuracy in identifying malicious network traffic
- Successfully detected: DoS attacks, exploits, reconnaissance, backdoors
- Demonstrated temporal pattern recognition in network behavior

**Tech Stack**
- Python, PyTorch, NumPy, Pandas
- Metaheuristic algorithms for feature selection
- UNSW-NB15 dataset (~250K network flows)

**What I Learned**
- Why LSTM works well for network traffic (temporal dependencies)
- Feature selection reduces overfitting and computational cost
- Trade-offs between model complexity and inference speed
- Unsupervised learning (autoencoder) works well when attack types are diverse

**Code**: Available in private repository (academic integrity - contains coursework)

---

## Static Site Generator
*C++17 Command-Line Tool | October 2024*

**Overview**

Built a minimal static site generator in modern C++ to learn the language beyond academics and automate my portfolio deployment.

**Features**
- Markdown → HTML conversion with custom parser
- Template system for consistent page layout
- Auto-generated navigation from content directory
- `<filesystem>` for recursive directory walking
- Regex-based parsing for Markdown syntax
- CLI tool with clean interface

**Deployment**
- GitHub Actions workflow for automatic builds
- Deploys to GitHub Pages on every push to main
- Zero-config deployment (just push Markdown)

**Technical Highlights**
- C++17 features: `std::filesystem`, `std::optional`, structured bindings
- String processing without external dependencies
- Cross-platform compatibility (Linux, macOS, Windows)
- Clean separation: parser → template engine → file writer

**What I Learned**
- Modern C++ best practices (RAII, smart pointers, move semantics)
- Building practical CLI tools
- CI/CD with GitHub Actions
- When to use C++ vs higher-level languages (this could've been Python, but I wanted C++ practice!)

**Why This Matters**
- Shows I can learn new languages independently
- Demonstrates ability to build developer tooling
- Production deployment (you're reading a page it generated!)

**Source**: [GitHub](https://github.com/loki52501) *(link to your repo)*

---

## Digital Library Platform
*Full-Stack Web Application | Feb - Aug 2024*

**Overview**

Built academic resource portal for Anna University to improve e-resource access for thousands of students.

**Architecture**
- **Backend**: Node.js with Express.js, RESTful API design
- **Database**: PostgreSQL with normalized schema
- **Frontend**: React with responsive design
- **Features**: User authentication, resource search, category browsing, admin dashboard

**Technical Decisions**
- PostgreSQL over MongoDB: Structured data with relationships (users, resources, categories)
- REST over GraphQL: Simpler for straightforward CRUD operations
- Server-side rendering for initial load performance

**Impact**
- Deployed for university use
- Serves academic papers, journals, e-books
- Improved discoverability of existing resources

**What I Learned**
- Full-stack development workflow
- Database design and normalization
- API design principles
- Deployment and hosting

---

## NFT Tracking Dashboard (Competition Win)
*Google Apps Script | April 2021*

**Challenge**

NIT Trichy Covalent Blockchain Competition: Build a functional blockchain application in 24 hours.

**Solution**

Built real-time NFT price tracking dashboard in Google Sheets using Apps Script:
- Learned Google Apps Script in one day (never used before)
- Integrated with blockchain APIs for live NFT data
- Auto-updating dashboard with price alerts
- Custom UI with Google Sheets as frontend

**Result**: 🥇 First Place

**What It Shows**
- Ability to learn new technologies rapidly under pressure
- Creative problem-solving (using Sheets as UI = accessible to non-technical users)
- API integration and data processing

---

## What I'm Building Next

After completing EdgeStreamAI (May 2026), I plan to:

1. **Contribute to Netflix Open Source**
   - Target: 2-3 meaningful PRs to projects like Hystrix, Vector, or network-related repos
   - Goal: Build relationship with Netflix engineering community

2. **Expand EdgeStreamAI Security Research**
   - Publish findings on AI security in operations environments
   - Create benchmark suite for testing AI robustness in infrastructure tools

3. **Learn Rust**
   - Build a simple network packet analyzer in Rust
   - Compare performance with Python implementations
   - Modern systems programming alternative to C++

---

## Technical Interests

Beyond these projects, I'm continuously learning:

**Reading**
- "Designing Data-Intensive Applications" by Martin Kleppmann (in progress)
- Netflix Tech Blog (streaming infrastructure deep dives)
- Security research papers on AI vulnerabilities

**Daily Practice**
- LeetCode problems (maintaining algorithm skills)
- System design exercises
- Following security advisories and CVEs

**Exploring**
- Kubernetes and container orchestration
- Distributed systems patterns (Raft, Paxos)
- Modern network protocols (QUIC, HTTP/3)

---

## Want to Collaborate?

If you're working on:
- Large-scale streaming infrastructure
- AI security and robustness
- Network security at scale
- Cloud security automation

I'd love to discuss technical approaches, trade-offs, and challenges. Reach out at **lokeshlks01@gmail.com**.

---

*Projects are ordered by recency and relevance to my career goals. All code is available upon request (some projects involve coursework and respect academic integrity policies).*
## (Optional) Research & Writing

### AI Collaboration: Shared vs. Individual Contexts

A research initiative on how groups collaborate with AI tools:

- Compares **shared group context** vs **individual context** when using LLMs  
- Looks at information sharing, dominance, and creativity in group problem-solving  
- Connects social-informatics theory with practical AI tool design

This work shows:

- Ability to **reason about complex socio-technical systems**  
- Comfort reading and synthesizing **research literature**  
- Interest in how tools shape **collaboration and cognition**

