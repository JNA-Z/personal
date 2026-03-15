import os
import chardet
import codecs

target_encoding = 'utf-8'

def detect_encoding(file_path):
    with open(file_path, 'rb') as f:
        result = chardet.detect(f.read())
        return result['encoding']

def convert_file_encoding(file_path, source_encoding, target_encoding):
    with codecs.open(file_path, 'r', source_encoding) as source_file:
        content = source_file.read()
    with codecs.open(file_path, 'w', target_encoding) as target_file:
        target_file.write(content)

def convert_directory(directory):
    for root, dirs, files in os.walk(directory):
        for filename in files:
            if filename.endswith('.c') or filename.endswith('.h'):
                file_path = os.path.join(root, filename)
                source_encoding = detect_encoding(file_path)
                print(f"Detected encoding '{source_encoding}' for file '{file_path}'. Converting to '{target_encoding}'...")
                convert_file_encoding(file_path, source_encoding, target_encoding)

# 替换为需要修文件的目录路径
directory_path = R'./src'
convert_directory(directory_path)
