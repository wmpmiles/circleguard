class AnticheatException(Exception):
    """Base class for exceptions in the anticheat program."""

class InvalidArgumentsException(AnticheatException):
    """Indicates an invalid argument was passed to one of the flags."""

class APIException(AnticheatException):
    """
    Indicates an error involving the API, which may or may not be fatal.

    UnkownAPIExceptions are considered fatal, InternalAPIExceptions are not.
    """

class UnkownAPIException(AnticheatException):
    """Indicates some error on the API's end that we were not prepared to handle."""


class InternalAPIException(APIException):
    """Indicates a response from the API that we know how to handle."""

class RetryException(InternalAPIException):
    """Indicates a response that means we should retry the request."""

class RatelimitException(RetryException):
    """Indicates that our key has been ratelimited and we should retry the request at a later date."""
