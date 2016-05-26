<?hh
namespace Ducts;

use HackPack\HackUnit\Contract\Assert;
use Psr\Http\Message\ServerRequestInterface as Request;
use Psr\Http\Message\ResponseInterface as Response;

function resolvingFunctionSecond(Request $a, Response $b, (function (Request,Response): Response) $c) {
    $a = $a->withAttribute('bar', 'baz');
    $r = $c($a, $b);
    return $r->withHeader('X-Out-Second', '1');
}

class ResolvingRunnerTest
{
    <<Test>>
    public async function testRun(Assert $assert): Awaitable<void>
    {
        $resolver = function (mixed $name): (function (Request,Response,(function (Request,Response): Response)): Response) use ($assert)  {
            if ($name === 'first') {
                return function (Request $a, Response $b, (function (Request,Response): Response) $c) use ($assert) {
                    $a = $a->withAttribute('foo', 'bar');
                    $r = $c($a, $b);
                    $assert->bool($r->hasHeader('X-Out-Second'))->is(true);
                    return $r->withHeader('X-Out-Third', '1');
                };
            }
            throw new \Exception();
        };
        $queue = [
            'first',
            'Ducts\resolvingFunctionSecond',
            function (Request $a, Response $b, (function (Request,Response): Response) $c) use ($assert) {
                $assert->mixed($a->getAttribute('bar'))->identicalTo('baz');
                $a = $a->withAttribute('baz', 'biz');
                $r = $c($a, $b);
                return $r->withHeader('X-Out-First', '1');
            },
            $this
        ];
        $runner = new ResolvingRunner($queue, $resolver);
        $request = new \Zend\Diactoros\ServerRequest();
        $response = new \Zend\Diactoros\Response();
        $out = $runner->run($request, $response);
        $assert->string($out->getHeaderLine('X-Out-Third'))->is("1");
    }

    public function __invoke(Request $request, Response $response, (function (Request,Response): Response) $next): Response
    {
        return $response->withHeader('X-Unit-Test', 'foobar');
    }
}
