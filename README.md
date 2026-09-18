# Zigistry API

- **To run this API service:**

```sh
git clone git@github.com:Zigistry/api.git
cd api
```

- **Then we need to install sqlite3 (with the FTS5 extension) for your platform**

```sh
brew install sqlite3

sudo dnf install -y sqlite-devel

make download_database
```

- **Build and run:**

```sh
make
./build/server
```

