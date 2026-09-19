FROM --platform=linux/amd64 amd64/fedora:latest

RUN dnf install -y curl make sqlite

RUN curl -fL "https://github.com/Zigistry/api/releases/download/api-binary/server" -o ./server

RUN curl -fL "https://huggingface.co/buckets/Zigistry/Zigistry/resolve/zigistry.db" -o ./zigistry.db

RUN chmod +x ./server

EXPOSE 7860

CMD ["./server"]