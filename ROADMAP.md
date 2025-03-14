A question emerges: use high/low level miniaudio API?
Get started use a hybrid, low level for copying pcm frames from stdin
    and putting them into a data source for high level premade nodes to process.
    This is somewhat seen in `hilo_interop` example.
Then there is the question of how to write pcm to stdout. I assume low level needs to receive and copy.
Eventually the whole should probably be implemented with the low level, meaning effects process will
be entirely custom.

Thinking about further development: a daemon to facilitate IPC: runtime updating of effects,
    ability to set up multiple input streams for a merger process, creation of a format to specify
    process configurations
