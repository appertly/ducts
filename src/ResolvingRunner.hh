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
 * A reusable execution through a queue of function layers that resolves layers.
 */
class ResolvingRunner extends Runner
{
    /**
     * Creates a new Runner.
     *
     * @param $queue - The queue of layers to execute
     * @param $resolver - A function to return a layer
     */
    public function __construct(Traversable<mixed> $queue, (function (mixed): (function (Request,Response,(function(Request,Response): Response)): Response)) $resolver)
    {
        $q = Vector{};
        foreach ($queue as $v) {
            if ($v instanceof \Closure) {
                $q[] = $v;
            } elseif (is_callable($v)) {
                $q[] = $v;
            } else {
                $q[] = $resolver($v);
            }
        }
        /* HH_IGNORE_ERROR[4110]: all values are callable */
        parent::__construct($q);
    }
}
