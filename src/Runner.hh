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
 * A reusable execution through a queue of function layers
 */
class Runner
{
    /**
     * A reversed queue of layers to execute.
     */
    protected ImmVector<(function (Request,Response,(function(Request,Response): Response)): Response)> $queue;

    /**
     * Creates a new Runner.
     *
     * @param $queue - The queue of layers to execute
     */
    public function __construct(Traversable<(function (Request,Response,(function(Request,Response): Response)): Response)> $queue)
    {
        $qv = new Vector($queue instanceof \ArrayObject ? $queue->getArrayCopy() : $queue);
        $qv->reverse();
        $this->queue = $qv->immutable();
    }

    /**
     * Runs through the queue, returning the response.
     *
     * @param $request - The PSR-7 HTTP Request
     * @param $response - The PSR-7 HTTP Response
     * @return - The resulting response
     */
    public function run(Request $request, Response $response): Response
    {
        $run = new Run($this->queue);
        return $run->__invoke($request, $response);
    }

    /**
     * Returns a middleware layer of this runner.
     *
     * For example, you declare this runner to be the second layer of another
     * runner. If this runner has three layers, it will participate in the
     * containing runner like this:
     * ```
     * |→ Outer runner layer 1
     * |  → Inner runner layer 1
     * |    → Inner runner layer 2
     * |      → Inner runner layer 3
     * |        → Outer runner layer 3
     * |        ← Outer runner layer 3
     * |      ← Inner runner layer 3
     * |    ← Inner runner layer 2
     * |  ← Inner runner layer 1
     * |← Outer runner layer 1
     * ```
     *
     * @return - This Runner as a middleware layer
     */
    public function layer(): (function (Request,Response,(function(Request,Response): Response)): Response)
    {
        return function (Request $req, Response $res, (function(Request,Response): Response) $next): Response {
            $run = new Run($this->queue, $next);
            return $run->__invoke($req, $res);
        };
    }
}
