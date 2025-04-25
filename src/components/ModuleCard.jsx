export default function ModuleCard({ module }) {
  return (
    <div className="bg-white rounded-2xl shadow p-4 transition hover:shadow-lg">
      <h2 className="text-xl font-semibold mb-2 text-indigo-700">{module.title}</h2>
      {module.sections.map((section, i) => (
        <div key={i} className="mb-3">
          <h3 className="font-medium text-gray-700">{section.heading}</h3>
          <ul className="list-disc list-inside text-sm text-gray-600">
            {section.points.map((pt, j) => <li key={j}>{pt}</li>)}
          </ul>
        </div>
      ))}
    </div>
  )
}
