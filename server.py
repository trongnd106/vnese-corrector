from flask import Flask, request, jsonify
from flask_cors import CORS
import re
import numpy as np
from nltk.util import ngrams
from collections import Counter
from keras.models import load_model
import string

app = Flask(__name__)
# app = Flask(__name__, static_folder='vendors')
CORS(app, resources={r"/*": {"origins": "*"}})

model = load_model("./model/spell_0.98.keras")

# Placeholder for constants and model (these need to be properly defined)
NGRAM=5
MAXLEN=40
alphabet = ['\x00', ' ', '_', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'á', 'à', 'ả', 'ã', 'ạ', 'â', 'ấ', 'ầ', 'ẩ', 'ẫ', 'ậ', 'ă', 'ắ', 'ằ', 'ẳ', 'ẵ', 'ặ', 'ó', 'ò', 'ỏ', 'õ', 'ọ', 'ô', 'ố', 'ồ', 'ổ', 'ỗ', 'ộ', 'ơ', 'ớ', 'ờ', 'ở', 'ỡ', 'ợ', 'é', 'è', 'ẻ', 'ẽ', 'ẹ', 'ê', 'ế', 'ề', 'ể', 'ễ', 'ệ', 'ú', 'ù', 'ủ', 'ũ', 'ụ', 'ư', 'ứ', 'ừ', 'ử', 'ữ', 'ự', 'í', 'ì', 'ỉ', 'ĩ', 'ị', 'ý', 'ỳ', 'ỷ', 'ỹ', 'ỵ', 'đ', 'Á', 'À', 'Ả', 'Ã', 'Ạ', 'Â', 'Ấ', 'Ầ', 'Ẩ', 'Ẫ', 'Ậ', 'Ă', 'Ắ', 'Ằ', 'Ẳ', 'Ẵ', 'Ặ', 'Ó', 'Ò', 'Ỏ', 'Õ', 'Ọ', 'Ô', 'Ố', 'Ồ', 'Ổ', 'Ỗ', 'Ộ', 'Ơ', 'Ớ', 'Ờ', 'Ở', 'Ỡ', 'Ợ', 'É', 'È', 'Ẻ', 'Ẽ', 'Ẹ', 'Ê', 'Ế', 'Ề', 'Ể', 'Ễ', 'Ệ', 'Ú', 'Ù', 'Ủ', 'Ũ', 'Ụ', 'Ư', 'Ứ', 'Ừ', 'Ử', 'Ữ', 'Ự', 'Í', 'Ì', 'Ỉ', 'Ĩ', 'Ị', 'Ý', 'Ỳ', 'Ỷ', 'Ỹ', 'Ỵ', 'Đ']
letters=list("abcdefghijklmnopqrstuvwxyzáàảãạâấầẩẫậăắằẳẵặóòỏõọôốồổỗộơớờởỡợéèẻẽẹêếềểễệúùủũụưứừửữựíìỉĩịýỳỷỹỵđABCDEFGHIJKLMNOPQRSTUVWXYZÁÀẢÃẠÂẤẦẨẪẬĂẮẰẲẴẶÓÒỎÕỌÔỐỒỔỖỘƠỚỜỞỠỢÉÈẺẼẸÊẾỀỂỄỆÚÙỦŨỤƯỨỪỬỮỰÍÌỈĨỊÝỲỶỸỴĐ")
accepted_char=list((string.digits + ''.join(letters)))

def extract_phrases(text):
    pattern = r'\w[\w ]*|\s\W+|\W+'
    return re.findall(pattern, text)

def encoder_data(text, maxlen=MAXLEN):
    text = "\x00" + text
    x = np.zeros((maxlen, len(alphabet)))
    for i, c in enumerate(text[:maxlen]):
        x[i, alphabet.index(c)] = 1
    if i < maxlen - 1:
        for j in range(i + 1, maxlen):
            x[j, 0] = 1
    return x

def decoder_data(x):
    x = x.argmax(axis=-1)
    return ''.join(alphabet[i] for i in x)

def nltk_ngrams(words, n=5):
    return ngrams(words.split(), n)

def guess(ngram):
    text = ' '.join(ngram)
    preds = model.predict(np.array([encoder_data(text)]), verbose=0)
    return decoder_data(preds[0]).strip('\x00')

def correct(sentence):
    for i in sentence:
        if i not in accepted_char:
            sentence = sentence.replace(i, " ")
    ngrams_list = list(nltk_ngrams(sentence, n=NGRAM))
    guessed_ngrams = list(guess(ngram) for ngram in ngrams_list)
    candidates = [Counter() for _ in range(len(guessed_ngrams) + NGRAM - 1)]
    for nid, ngram in enumerate(guessed_ngrams):
        for wid, word in enumerate(re.split(' +', ngram)):
            if nid + wid < len(candidates):
                candidates[nid + wid].update([word])
            else:
                candidates.extend([Counter() for _ in range(nid + wid - len(candidates) + 1)])
                candidates[nid + wid].update([word])
    output = ' '.join(c.most_common(1)[0][0] if c.most_common(1) else '' for c in candidates)
    return output

@app.route('/correct', methods=['POST'])
def correct_text():
    try:
        data = request.get_json()
        if not data or 'text' not in data:
            return jsonify({'error': 'Missing text field in request body'}), 400
        
        input_text = data['text']
        if not isinstance(input_text, str):
            return jsonify({'error': 'Text field must be a string'}), 400

        corrected_text = correct(input_text)
        return jsonify({'corrected_text': corrected_text}), 200

    except Exception as e:
        return jsonify({'error': str(e)}), 500

if __name__ == '__main__':
    app.run(debug=False, host='0.0.0.0', port=8000)