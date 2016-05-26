<?hh // strict
/**
 * Ducts
 *
 * @copyright 2016 Appertly contributors
 * @license   MIT
 */
namespace Ducts;

use Psr\Http\Message\ServerRequestInterface as Request;
use Psr\Http\Message\ResponseInterface as Response;

/**
 * A single run-through of a queue of request–response layers.
 */
class Run
{
    private Vector<(function (Request,Response,(function(Request,Response): Response)): Response)> $queue;
    private (function(Request,Response): Response) $last;

    /**
     * Creates a new Run.
     *
     * @param $queue - A reversed `ImmVector` containing all the layers to include in the run
     * @param $last - Optional. The last layer in the queue; the innermost layer.
     */
    public function __construct(ImmVector<(function (Request,Response,(function(Request,Response): Response)): Response)> $reversedQueue, ?(function(Request,Response): Response) $last = null)
    {
        $this->queue = $reversedQueue->toVector();
        $this->last = $last ?: class_meth(__CLASS__, 'responseIdentity');
    }

    /**
     * Invokes this class.
     *
     * @param $request - The PSR-7 HTTP request
     * @param $response - The PSR-7 HTTP response
     * @return - The resulting response
     */
    public function __invoke(Request $request, Response $response): Response
    {
        if ($this->queue->isEmpty()) {
            $c = $this->last;
            return $c($request, $response);
        } else {
            $c = $this->queue->pop();
            return $c($request, $response, inst_meth($this, '__invoke'));
        }
    }

    /**
     * Convenience static method that returns the response given.
     *
     * @param $request - The PSR-7 HTTP request
     * @param $response - The PSR-7 HTTP response
     * @return - Returns the `$response` provided
     */
    public static function responseIdentity(Request $request, Response $response): Response
    {
        return $response;
    }
}
