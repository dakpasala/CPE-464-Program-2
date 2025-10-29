# CPE 464 Tic-Tac-Toe Docker Testing Setup

This guide explains how to set up Docker testing for your client and server implementations.

## Prerequisites

1. **Install Docker Desktop** (includes Docker Compose):
   - **Windows/Mac**: https://docs.docker.com/get-docker/
   - **Linux**:
     ```bash
     # Ubuntu/Debian
     sudo apt-get update
     sudo apt-get install docker.io docker-compose
     sudo usermod -aG docker $USER
     # Log out and back in for group changes to take effect
     ```

## Setup (One-Time)

### Step 1: Get Access to Images

1. **Pull the reference images**:
   ```bash
   docker pull prschmitt/cpe464-ttt-server:spring2025
   docker pull prschmitt/cpe464-ttt-client:spring2025
   ```

2. **Verify images downloaded**:
   ```bash
   docker images | grep cpe464-ttt
   ```

   You should see both images listed (~75 MB each).

### Step 2: Download Testing Files

Download these files from Canvas or the course website:
- `docker-compose.yml`
- This README

## Testing Your Implementation

### Option A: Test Your Client

Your client should work with the reference server.

1. **Start the reference server**:
   Note docker-compose automatically searches for `docker-compose.yml` in the current directory.
   ```bash
   docker-compose up -d ref-server
   ```

2. **Test your client**:
   ```bash
   # Most systems (Linux/Windows/Mac):
   ./your-ttt-client <your_username> 127.0.0.1 15464

   # Or try localhost if that doesn't work:
   ./your-ttt-client <your_username> localhost 15464
   ```

3. **Stop the server when done**:
   ```bash
   docker-compose down
   ```

### Option B: Test Your Server

Your server should work with the reference client.

1. **Start your server**:
   ```bash
   ./your-ttt-server 15464 &
   SERVER_PID=$!
   ```

2. **Test with reference client**:
   ```bash
   docker-compose run --rm ref-client test_user host.docker.internal 15464 
   ```

3. **Stop your server**:
   ```bash
   kill $SERVER_PID
   # Or: pkill ttt-server
   ```

## Commands Reference

### Client Commands
Inside the client, you can use:
- `list` or `l` - Show players online
- `play <username>` or `p <username>` - Start a game
- `move <1-9>` or `m <1-9>` or just `<1-9>` - Make a move
- `quit` or `q` - Exit client

### Docker Commands

**Start reference server**:
```bash
docker-compose up -d ref-server
```

**Stop everything**:
```bash
docker-compose down
```

**Run reference client**:
```bash
docker-compose run --rm ref-client <username> <server> <port>
```

**Check running containers**:
```bash
docker ps
```

## Troubleshooting

### "Cannot connect to the Docker daemon"

Docker isn't running. Start Docker Desktop (Windows/Mac) or:
```bash
# Linux
sudo systemctl start docker
```

### "port 15464 already in use"

Something else is using the port:
```bash
# Stop the reference server
docker-compose down

# Or kill your own server
pkill ttt-server

# Check what's using the port
lsof -i :15464
```

### "Connection refused" or localhost doesn't work

**Solution**: Use `127.0.0.1` instead of `localhost`:
```bash
./your-ttt-client 127.0.0.1 15464 username
```

The docker-compose.yml is configured to bind to 127.0.0.1, which works reliably across all platforms (Linux/Mac/Windows).

### "host.docker.internal: Name or service not known" (Linux)

This is already handled in the docker-compose.yml with:
```yaml
extra_hosts:
  - "host.docker.internal:host-gateway"
```

If it still doesn't work, use your machine's IP instead:
```bash
# Find your IP
hostname -I | awk '{print $1}'

# Use it instead of host.docker.internal
docker-compose run --rm ref-client 192.168.1.x 15464 test_user
```

### Client doesn't respond to Ctrl+C

If the client is stuck, force stop it:
```bash
# In another terminal
docker ps  # Find the container ID
docker stop <container_id>
```


for personal notes:
gcc -o ttt-client client.c pdu.c
gcc -o ttt-server server.c pdu.c game.c users.c

