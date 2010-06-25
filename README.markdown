itimer
=======

The `itimer` module adds support for sub-second alarm signal delivery
on systems that support `setitimer(2)` and `getitimer(2)`. Usage is
very similar to sending alarm signals with the built-in `signal` module:

    import itimer
    import signal
    import time

    def handler(*args, **kwargs):
        print 'Alarm! (%s)' % time.time()

    def main():
        signal.signal(signal.SIGALRM, handler)

        cont = True
        while cont:
            try:
                time.sleep(1)
                itimer.alarm(0.5)
            except KeyboardInterrupt:
                cont = False
        return 0

    if __name__ == '__main__':
        exit(main())

(The example above will fire SIGALRM signals every 0.5s)


License
========

See the `LICENSE` file


Authors
=========

`itimer` was originally written by Libor Michalek at [Slide, Inc.](http://github.com/slideinc)
