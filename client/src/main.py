import argparse
from connection_manager import handle_connection

def main():
    parser = argparse.ArgumentParser(description="Run the connection manager with specified IP and port.")

    parser.add_argument(
        "-ip", 
        type=str, 
        default="127.0.0.1", 
        help="The IP address of the server. Default is 127.0.0.1."
    )

    parser.add_argument(
        "-port", 
        type=int, 
        default=21, 
        help="The port of the server. Default is 21."
    )

    args = parser.parse_args()

    handle_connection(args.ip, args.port)

if __name__ == "__main__":
    main()
