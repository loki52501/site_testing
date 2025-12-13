# AI-Driven Streaming Infra: Keeping Playback Smooth
*Published on 2025-11-20*

I built an adaptive streaming platform that uses AI-based bitrate prediction to reduce buffering when networks fluctuate. This was amazing.

## Stack
- Python + FastAPI on AWS EC2
- React client with SSE for real-time token delivery
- Frame compression tuned for low-latency playback

## What Worked
- AI bitrate prediction reduced buffer events compared to static ABR.
- SSE kept clients in sync without heavy polling.
- Compression tweaks cut glass-to-glass delay.

## Takeaways
- Prediction models should be retrained per region; network shape matters.
- Server hints plus client heuristics beat either alone.
