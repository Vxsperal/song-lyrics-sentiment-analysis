import pandas as pd
import re
from langdetect import detect
from transformers import BertTokenizer, BertForSequenceClassification, Trainer, TrainingArguments
from datasets import Dataset
import torch
import numpy as np
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import LabelEncoder
from sklearn.metrics import precision_score, f1_score, confusion_matrix, ConfusionMatrixDisplay
import matplotlib.pyplot as plt
import os

os.environ["WANDB_DISABLED"] = "true"

# Load dataset
df = pd.read_csv('../data/Song_DataSet.csv')

# Clean [Intro], [Chorus], etc.
def remove_bracket_tags(text):
    return re.sub(r'\[.*?\]', '', text)

# Detect English
def is_english(text):
    try:
        return detect(text) == 'en'
    except:
        return False

# Drop NaNs
df = df.dropna(subset=['Song Lyrics', 'Song Sentiment Tag'])

# Map 5 tags to 3 classes
def map_to_three_classes(tag):
    tag = tag.lower().strip()
    if tag in ['very happy', 'happy']:
        return 'Happy'
    elif tag in ['very sad', 'sad']:
        return 'Sad'
    else:
        return 'Neutral'

df['Mapped Sentiment'] = df['Song Sentiment Tag'].apply(map_to_three_classes)

# Remove bracket tags and filter for English lyrics
df['Song Lyrics'] = df['Song Lyrics'].apply(remove_bracket_tags)
df = df[df['Song Lyrics'].apply(lambda x: is_english(x) and len(x.split()) > 10)]

# Encode labels
label_encoder = LabelEncoder()
df['label'] = label_encoder.fit_transform(df['Mapped Sentiment'])

# Stratified split into train/validation
train_texts, val_texts, train_labels, val_labels = train_test_split(
    df['Song Lyrics'].tolist(),
    df['label'].tolist(),
    test_size=0.2,
    stratify=df['label'],
    random_state=42
)

# Tokenization
tokenizer = BertTokenizer.from_pretrained('bert-base-uncased')
print("Class distribution:\n", df['Mapped Sentiment'].value_counts())

def tokenize(batch):
    return tokenizer(batch['text'], padding='max_length', truncation=True, max_length=512)

train_dataset = Dataset.from_dict({'text': train_texts, 'label': train_labels}).map(tokenize, batched=True)
val_dataset = Dataset.from_dict({'text': val_texts, 'label': val_labels}).map(tokenize, batched=True)

train_dataset.set_format(type='torch', columns=['input_ids', 'attention_mask', 'label'])
val_dataset.set_format(type='torch', columns=['input_ids', 'attention_mask', 'label'])

# Load model with 3 output labels
model = BertForSequenceClassification.from_pretrained('bert-base-uncased', num_labels=3)

# Training arguments
training_args = TrainingArguments(
    output_dir='./results',
    do_train=True,
    do_eval=True,
    learning_rate=2e-5,
    per_device_train_batch_size=16,
    per_device_eval_batch_size=16,
    num_train_epochs=8,
    weight_decay=0.01,
    logging_dir='./logs',
    logging_steps=50,
    save_steps=500,
    eval_steps=200,
    report_to="none"
)

# Metrics
def compute_metrics(eval_pred):
    logits, labels = eval_pred
    predictions = np.argmax(logits, axis=-1)
    precision = precision_score(labels, predictions, average='weighted')
    f1 = f1_score(labels, predictions, average='weighted')
    acc = np.mean(predictions == labels)
    return {
        'accuracy': acc,
        'precision': precision,
        'f1': f1
    }

# Trainer
trainer = Trainer(
    model=model,
    args=training_args,
    train_dataset=train_dataset,
    eval_dataset=val_dataset,
    compute_metrics=compute_metrics
)

# Train model
trainer.train()

# Evaluate model
from tabulate import tabulate
final_metrics = trainer.evaluate()
metrics_table = [
    ["Accuracy", final_metrics['eval_accuracy']],
    ["Precision", final_metrics['eval_precision']],
    ["F1 Score", final_metrics['eval_f1']]
]
print("\nMetrics Summary:")
print(tabulate(metrics_table, headers=["Metric", "Value"], tablefmt="grid"))

# Confusion matrix
predictions = trainer.predict(val_dataset)
y_true = predictions.label_ids
y_pred = np.argmax(predictions.predictions, axis=-1)

cm = confusion_matrix(y_true, y_pred)
disp = ConfusionMatrixDisplay(confusion_matrix=cm, display_labels=label_encoder.classes_)

plt.figure(figsize=(8, 6))
disp.plot(cmap=plt.cm.Blues, xticks_rotation=45)
plt.title("Confusion Matrix")
plt.tight_layout()
plt.show()
