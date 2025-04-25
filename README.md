# song-lyrics-sentiment-analysis
NLP project on sentiment analysis of song lyrics
# Song Lyrics Sentiment Analysis (NLP Project)

This project explores sentiment analysis on song lyrics by combining:
- C++ scraping of mood metadata from Spotify
- Python-based lyrics processing and NLP model training

## Structure
- `/scraper_cpp/`: C++ scraper that extracts song mood features (valence, energy)
- `/nlp_pipeline/`: Python scripts for lyrics collection, preprocessing, and model training
- `/data/`: CSV dataset combining mood metadata and song lyrics


---

## Running the Python Pipeline

> Assumes `data/lyrics_data.csv` has already been generated using the C++ scraper.

### 1. Install Dependencies

```bash
pip install -r requirements.txt
